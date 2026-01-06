/**
 * @file lora_mesh.cpp
 * @brief Council of Ricks - LoRa Mesh Implementation
 */

#include "lora_mesh.h"
#include <esp_system.h>

// =============================================================================
// HARDWARE CONFIGURATION (K257 SX1262)
// =============================================================================
#define LORA_CS     7
#define LORA_DIO1   9
#define LORA_RST    8
#define LORA_BUSY   36

// RadioLib SX1262 instance
static SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

// Receive buffer
static volatile bool receivedFlag = false;
static mesh_message_t rxMessage;

// =============================================================================
// ISR CALLBACK
// =============================================================================
static void IRAM_ATTR onReceive(void) {
    receivedFlag = true;
}

// =============================================================================
// INITIALIZATION
// =============================================================================
bool lora_mesh_init(lora_mesh_state_t* state) {
    Serial.println("[LoRa] Initializing SX1262...");

    // Generate device ID from ESP32 MAC
    uint64_t mac = ESP.getEfuseMac();
    memcpy(state->deviceId, &mac, 6);

    // Set default device name
    snprintf(state->deviceName, sizeof(state->deviceName), "Rick-%02X%02X",
             state->deviceId[4], state->deviceId[5]);

    // Initialize radio
    int status = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR,
                             LORA_SYNC, LORA_POWER, LORA_PREAMBLE);

    if (status != RADIOLIB_ERR_NONE) {
        Serial.printf("[LoRa] Init failed: %d\n", status);
        state->initialized = false;
        return false;
    }

    // Set DIO1 as interrupt
    radio.setDio1Action(onReceive);

    // Start receiving
    radio.startReceive();

    // Initialize state
    state->initialized = true;
    state->enabled = false;
    state->nodeCount = 0;
    state->lastBeacon = 0;
    state->msgSent = 0;
    state->msgReceived = 0;

    Serial.printf("[LoRa] Initialized as %s\n", state->deviceName);
    return true;
}

// =============================================================================
// ENABLE/DISABLE
// =============================================================================
void lora_mesh_enable(lora_mesh_state_t* state, bool enable) {
    if (!state->initialized) return;

    state->enabled = enable;
    if (enable) {
        radio.startReceive();
        lora_mesh_send_beacon(state);
        Serial.println("[LoRa] Mesh enabled");
    } else {
        radio.standby();
        Serial.println("[LoRa] Mesh disabled");
    }
}

// =============================================================================
// UPDATE LOOP
// =============================================================================
void lora_mesh_update(lora_mesh_state_t* state) {
    if (!state->initialized || !state->enabled) return;

    // Send beacon periodically
    if (millis() - state->lastBeacon > BEACON_INTERVAL_MS) {
        lora_mesh_send_beacon(state);
    }

    // Check for received messages
    if (receivedFlag) {
        receivedFlag = false;

        uint8_t buf[256];
        int len = radio.getPacketLength();

        if (len > 0 && len <= sizeof(buf)) {
            int status = radio.readData(buf, len);

            if (status == RADIOLIB_ERR_NONE) {
                state->lastRssi = radio.getRSSI();
                state->lastSnr = radio.getSNR();
                state->msgReceived++;

                // Parse message
                if (len >= sizeof(mesh_message_t) - MAX_MSG_SIZE) {
                    mesh_message_t* msg = (mesh_message_t*)buf;

                    // Process based on type
                    switch (msg->type) {
                        case MSG_BEACON:
                            // Add/update node
                            {
                                bool found = false;
                                for (int i = 0; i < state->nodeCount; i++) {
                                    if (memcmp(state->nodes[i].id, msg->srcId, 6) == 0) {
                                        state->nodes[i].rssi = state->lastRssi;
                                        state->nodes[i].lastSeen = millis();
                                        found = true;
                                        break;
                                    }
                                }

                                if (!found && state->nodeCount < MAX_MESH_NODES) {
                                    mesh_node_t* node = &state->nodes[state->nodeCount];
                                    memcpy(node->id, msg->srcId, 6);
                                    memcpy(node->name, msg->data, min((int)msg->dataLen, 15));
                                    node->name[15] = 0;
                                    node->rssi = state->lastRssi;
                                    node->lastSeen = millis();
                                    node->handshakes = 0;
                                    state->nodeCount++;
                                    Serial.printf("[LoRa] New node: %s (RSSI: %d)\n",
                                                  node->name, node->rssi);
                                }
                            }
                            break;

                        case MSG_PING:
                            // Respond with pong
                            {
                                mesh_message_t pong;
                                pong.type = MSG_PONG;
                                memcpy(pong.srcId, state->deviceId, 6);
                                memcpy(pong.dstId, msg->srcId, 6);
                                pong.seqNum = msg->seqNum;
                                pong.dataLen = 0;
                                radio.transmit((uint8_t*)&pong, sizeof(pong) - MAX_MSG_SIZE);
                                radio.startReceive();
                            }
                            break;

                        case MSG_HANDSHAKE:
                            Serial.printf("[LoRa] Received handshake (%d bytes)\n", msg->dataLen);
                            // TODO: Save to SD card
                            break;

                        case MSG_CHAT:
                            Serial.printf("[LoRa] Chat: %.*s\n", msg->dataLen, msg->data);
                            break;
                    }
                }
            }
        }

        // Restart receive
        radio.startReceive();
    }

    // Prune stale nodes (not seen in 2 minutes)
    for (int i = state->nodeCount - 1; i >= 0; i--) {
        if (millis() - state->nodes[i].lastSeen > 120000) {
            Serial.printf("[LoRa] Node timeout: %s\n", state->nodes[i].name);
            // Shift remaining nodes
            for (int j = i; j < state->nodeCount - 1; j++) {
                state->nodes[j] = state->nodes[j + 1];
            }
            state->nodeCount--;
        }
    }
}

// =============================================================================
// SEND BEACON
// =============================================================================
bool lora_mesh_send_beacon(lora_mesh_state_t* state) {
    if (!state->initialized || !state->enabled) return false;

    mesh_message_t msg;
    msg.type = MSG_BEACON;
    memcpy(msg.srcId, state->deviceId, 6);
    memset(msg.dstId, 0xFF, 6);  // Broadcast
    msg.seqNum = 0;
    msg.dataLen = strlen(state->deviceName);
    memcpy(msg.data, state->deviceName, msg.dataLen);

    int status = radio.transmit((uint8_t*)&msg,
                                 sizeof(msg) - MAX_MSG_SIZE + msg.dataLen);

    radio.startReceive();
    state->lastBeacon = millis();

    if (status == RADIOLIB_ERR_NONE) {
        state->msgSent++;
        return true;
    }
    return false;
}

// =============================================================================
// SHARE HANDSHAKE
// =============================================================================
bool lora_mesh_share_handshake(lora_mesh_state_t* state, const uint8_t* data, size_t len) {
    if (!state->initialized || !state->enabled) return false;
    if (len > MAX_MSG_SIZE) return false;

    mesh_message_t msg;
    msg.type = MSG_HANDSHAKE;
    memcpy(msg.srcId, state->deviceId, 6);
    memset(msg.dstId, 0xFF, 6);  // Broadcast
    msg.seqNum = 0;
    msg.dataLen = len;
    memcpy(msg.data, data, len);

    int status = radio.transmit((uint8_t*)&msg,
                                 sizeof(msg) - MAX_MSG_SIZE + len);

    radio.startReceive();

    if (status == RADIOLIB_ERR_NONE) {
        state->msgSent++;
        Serial.printf("[LoRa] Shared handshake (%d bytes)\n", len);
        return true;
    }
    return false;
}

// =============================================================================
// SEND CHAT
// =============================================================================
bool lora_mesh_send_chat(lora_mesh_state_t* state, const char* message) {
    if (!state->initialized || !state->enabled) return false;

    size_t len = strlen(message);
    if (len > MAX_MSG_SIZE) len = MAX_MSG_SIZE;

    mesh_message_t msg;
    msg.type = MSG_CHAT;
    memcpy(msg.srcId, state->deviceId, 6);
    memset(msg.dstId, 0xFF, 6);  // Broadcast
    msg.seqNum = 0;
    msg.dataLen = len;
    memcpy(msg.data, message, len);

    int status = radio.transmit((uint8_t*)&msg,
                                 sizeof(msg) - MAX_MSG_SIZE + len);

    radio.startReceive();

    if (status == RADIOLIB_ERR_NONE) {
        state->msgSent++;
        return true;
    }
    return false;
}

// =============================================================================
// PING
// =============================================================================
bool lora_mesh_ping(lora_mesh_state_t* state, uint8_t nodeIndex) {
    if (!state->initialized || !state->enabled) return false;
    if (nodeIndex >= state->nodeCount) return false;

    mesh_message_t msg;
    msg.type = MSG_PING;
    memcpy(msg.srcId, state->deviceId, 6);
    memcpy(msg.dstId, state->nodes[nodeIndex].id, 6);
    msg.seqNum = millis() & 0xFF;
    msg.dataLen = 0;

    int status = radio.transmit((uint8_t*)&msg, sizeof(msg) - MAX_MSG_SIZE);

    radio.startReceive();

    if (status == RADIOLIB_ERR_NONE) {
        state->msgSent++;
        return true;
    }
    return false;
}

// =============================================================================
// GETTERS
// =============================================================================
uint8_t lora_mesh_get_node_count(lora_mesh_state_t* state) {
    return state->nodeCount;
}

mesh_node_t* lora_mesh_get_node(lora_mesh_state_t* state, uint8_t index) {
    if (index >= state->nodeCount) return nullptr;
    return &state->nodes[index];
}

bool lora_mesh_message_available(void) {
    return receivedFlag;
}

bool lora_mesh_get_message(mesh_message_t* msg) {
    if (!receivedFlag) return false;
    memcpy(msg, &rxMessage, sizeof(mesh_message_t));
    return true;
}
