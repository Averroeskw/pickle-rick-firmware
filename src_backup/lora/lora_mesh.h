/**
 * @file lora_mesh.h
 * @brief Council of Ricks - LoRa Mesh Network
 *
 * Share captures and data between Pickle Rick devices via LoRa
 */

#ifndef LORA_MESH_H
#define LORA_MESH_H

#include <Arduino.h>
#include <RadioLib.h>

// =============================================================================
// LORA CONFIGURATION
// =============================================================================
#define LORA_FREQ           915.0   // MHz (US frequency)
#define LORA_BW             125.0   // kHz bandwidth
#define LORA_SF             9       // Spreading factor
#define LORA_CR             7       // Coding rate 4/7
#define LORA_SYNC           0x12    // Sync word
#define LORA_POWER          20      // dBm (max for SX1262)
#define LORA_PREAMBLE       8       // Preamble length

// Message types
#define MSG_BEACON          0x01
#define MSG_HANDSHAKE       0x02
#define MSG_NETWORK_LIST    0x03
#define MSG_CHAT            0x04
#define MSG_ACK             0x05
#define MSG_PING            0x06
#define MSG_PONG            0x07

// Limits
#define MAX_MESH_NODES      16
#define MAX_MSG_SIZE        200
#define BEACON_INTERVAL_MS  30000

// =============================================================================
// DATA STRUCTURES
// =============================================================================

typedef struct {
    uint8_t id[6];          // Unique device ID
    char name[16];          // Device name
    int16_t rssi;           // Signal strength
    uint32_t lastSeen;      // Last seen timestamp
    uint16_t handshakes;    // Handshakes shared
    uint8_t rank;           // Rick rank
} mesh_node_t;

typedef struct {
    uint8_t type;           // Message type
    uint8_t srcId[6];       // Source device ID
    uint8_t dstId[6];       // Destination (0xFF = broadcast)
    uint8_t seqNum;         // Sequence number
    uint8_t dataLen;        // Data length
    uint8_t data[MAX_MSG_SIZE];
} mesh_message_t;

typedef struct {
    bool initialized;
    bool enabled;
    uint8_t deviceId[6];
    char deviceName[16];
    mesh_node_t nodes[MAX_MESH_NODES];
    uint8_t nodeCount;
    uint32_t lastBeacon;
    uint32_t msgSent;
    uint32_t msgReceived;
    int16_t lastRssi;
    float lastSnr;
} lora_mesh_state_t;

// =============================================================================
// FUNCTIONS
// =============================================================================

/**
 * Initialize LoRa mesh
 */
bool lora_mesh_init(lora_mesh_state_t* state);

/**
 * Enable/disable mesh
 */
void lora_mesh_enable(lora_mesh_state_t* state, bool enable);

/**
 * Update mesh - call in loop
 */
void lora_mesh_update(lora_mesh_state_t* state);

/**
 * Send beacon
 */
bool lora_mesh_send_beacon(lora_mesh_state_t* state);

/**
 * Send handshake data to mesh
 */
bool lora_mesh_share_handshake(lora_mesh_state_t* state, const uint8_t* data, size_t len);

/**
 * Send chat message
 */
bool lora_mesh_send_chat(lora_mesh_state_t* state, const char* message);

/**
 * Send ping to specific node
 */
bool lora_mesh_ping(lora_mesh_state_t* state, uint8_t nodeIndex);

/**
 * Get node count
 */
uint8_t lora_mesh_get_node_count(lora_mesh_state_t* state);

/**
 * Get node info
 */
mesh_node_t* lora_mesh_get_node(lora_mesh_state_t* state, uint8_t index);

/**
 * Check if message available
 */
bool lora_mesh_message_available(void);

/**
 * Get received message
 */
bool lora_mesh_get_message(mesh_message_t* msg);

#endif // LORA_MESH_H
