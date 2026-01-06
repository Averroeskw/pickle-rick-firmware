/**
 * @file wifi_scanner.cpp
 * @brief WiFi Scanner - Portal Gun Mode Implementation
 */

#include "wifi_scanner.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>

// =============================================================================
// SCANNER INITIALIZATION
// =============================================================================
bool scanner_init(scanner_state_t* state, uint16_t max_networks) {
    // Try PSRAM first, fallback to regular heap
    state->networks = (network_info_t*)ps_malloc(sizeof(network_info_t) * max_networks);
    if (!state->networks) {
        Serial.println("[SCANNER] PSRAM alloc failed, trying heap...");
        state->networks = (network_info_t*)malloc(sizeof(network_info_t) * max_networks);
    }
    if (!state->networks) {
        Serial.println("[SCANNER] Failed to allocate network buffer");
        // Initialize with zero capacity but don't fail - scanner can work without storage
        state->count = 0;
        state->capacity = 0;
        state->currentChannel = 1;
        state->isScanning = false;
        state->isHopping = true;
        state->scanStartTime = 0;
        state->lastHopTime = 0;
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        Serial.println("[SCANNER] Initialized in limited mode (no storage)");
        return true;  // Return true but with limited functionality
    }

    state->count = 0;
    state->capacity = max_networks;
    state->currentChannel = 1;
    state->isScanning = false;
    state->isHopping = true;
    state->scanStartTime = 0;
    state->lastHopTime = 0;

    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.printf("[SCANNER] Initialized with capacity for %d networks\n", max_networks);
    return true;
}

// =============================================================================
// START/STOP SCANNING
// =============================================================================
void scanner_start(scanner_state_t* state) {
    state->isScanning = true;
    state->scanStartTime = millis();
    state->lastHopTime = millis();

    // Set to first channel
    esp_wifi_set_channel(state->currentChannel, WIFI_SECOND_CHAN_NONE);

    Serial.println("[SCANNER] Portal Gun activated - Scanning dimensions...");
}

void scanner_stop(scanner_state_t* state) {
    state->isScanning = false;
    Serial.printf("[SCANNER] Portal closed. Found %d networks.\n", state->count);
}

// =============================================================================
// CHANNEL CONTROL
// =============================================================================
void scanner_set_channel(scanner_state_t* state, uint8_t channel) {
    if (channel >= WIFI_CHANNEL_MIN && channel <= WIFI_CHANNEL_MAX) {
        state->currentChannel = channel;
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    }
}

void scanner_set_hopping(scanner_state_t* state, bool enabled) {
    state->isHopping = enabled;
}

// =============================================================================
// SCANNER TICK (CALL IN LOOP)
// =============================================================================
void scanner_tick(scanner_state_t* state) {
    if (!state->isScanning) return;

    // Channel hopping
    if (state->isHopping && (millis() - state->lastHopTime > CHANNEL_HOP_INTERVAL_MS)) {
        state->lastHopTime = millis();
        state->currentChannel++;
        if (state->currentChannel > WIFI_CHANNEL_MAX) {
            state->currentChannel = WIFI_CHANNEL_MIN;
        }
        esp_wifi_set_channel(state->currentChannel, WIFI_SECOND_CHAN_NONE);
    }

    // Perform scan
    int n = WiFi.scanNetworks(true, true, false, 100);  // Async, show hidden, passive, 100ms per channel

    if (n == WIFI_SCAN_RUNNING) {
        return;  // Still scanning
    }

    if (n > 0) {
        for (int i = 0; i < n; i++) {
            // Check if network already exists
            network_info_t* existing = scanner_find_bssid(state, WiFi.BSSID(i));

            if (existing) {
                // Update existing entry
                existing->rssi = WiFi.RSSI(i);
                existing->lastSeen = millis();
            } else if (state->count < state->capacity) {
                // Add new network
                network_info_t net;
                memcpy(net.bssid, WiFi.BSSID(i), 6);
                strncpy(net.ssid, WiFi.SSID(i).c_str(), 32);
                net.ssid[32] = '\0';
                net.rssi = WiFi.RSSI(i);
                net.channel = WiFi.channel(i);
                net.authmode = WiFi.encryptionType(i);
                net.hidden = (strlen(net.ssid) == 0);
                net.firstSeen = millis();
                net.lastSeen = millis();
                net.latitude = 0;
                net.longitude = 0;
                net.hasHandshake = false;
                net.hasPMKID = false;

                scanner_add_network(state, &net);

                Serial.printf("[SCANNER] Found: %s [%02X:%02X:%02X:%02X:%02X:%02X] CH:%d RSSI:%d\n",
                              net.hidden ? "<hidden>" : net.ssid,
                              net.bssid[0], net.bssid[1], net.bssid[2],
                              net.bssid[3], net.bssid[4], net.bssid[5],
                              net.channel, net.rssi);
            }
        }
    }

    WiFi.scanDelete();
}

// =============================================================================
// NETWORK MANAGEMENT
// =============================================================================
network_info_t* scanner_get_network(scanner_state_t* state, uint16_t index) {
    if (index >= state->count) return nullptr;
    return &state->networks[index];
}

network_info_t* scanner_find_bssid(scanner_state_t* state, const uint8_t* bssid) {
    for (uint16_t i = 0; i < state->count; i++) {
        if (memcmp(state->networks[i].bssid, bssid, 6) == 0) {
            return &state->networks[i];
        }
    }
    return nullptr;
}

void scanner_add_network(scanner_state_t* state, network_info_t* network) {
    if (state->count >= state->capacity) return;
    memcpy(&state->networks[state->count], network, sizeof(network_info_t));
    state->count++;
}

void scanner_clear(scanner_state_t* state) {
    state->count = 0;
    Serial.println("[SCANNER] Network list cleared");
}

// =============================================================================
// EXPORT
// =============================================================================
bool scanner_export_wigle(scanner_state_t* state, const char* filename) {
    // TODO: Implement WiGLE CSV export
    Serial.printf("[SCANNER] Exporting %d networks to %s\n", state->count, filename);
    return true;
}

// =============================================================================
// STATISTICS
// =============================================================================
uint16_t scanner_count_by_auth(scanner_state_t* state, wifi_auth_mode_t auth) {
    uint16_t count = 0;
    for (uint16_t i = 0; i < state->count; i++) {
        if (state->networks[i].authmode == auth) count++;
    }
    return count;
}

uint16_t scanner_count_hidden(scanner_state_t* state) {
    uint16_t count = 0;
    for (uint16_t i = 0; i < state->count; i++) {
        if (state->networks[i].hidden) count++;
    }
    return count;
}

// =============================================================================
// PROMISCUOUS MODE
// =============================================================================
static wifi_promiscuous_cb_t userCallback = nullptr;

void scanner_enable_promisc(scanner_state_t* state) {
    esp_wifi_set_promiscuous(true);
    Serial.println("[SCANNER] Promiscuous mode enabled");
}

void scanner_disable_promisc(scanner_state_t* state) {
    esp_wifi_set_promiscuous(false);
    Serial.println("[SCANNER] Promiscuous mode disabled");
}

void scanner_set_callback(wifi_promiscuous_cb_t callback) {
    userCallback = callback;
    esp_wifi_set_promiscuous_rx_cb(callback);
}

// =============================================================================
// MAC RANDOMIZATION
// =============================================================================
static uint8_t originalMac[6];
static bool macSaved = false;

void scanner_randomize_mac() {
    if (!macSaved) {
        esp_wifi_get_mac(WIFI_IF_STA, originalMac);
        macSaved = true;
    }

    uint8_t newMac[6];
    newMac[0] = 0x02;  // Locally administered, unicast
    for (int i = 1; i < 6; i++) {
        newMac[i] = random(0, 256);
    }

    esp_wifi_set_mac(WIFI_IF_STA, newMac);
    Serial.printf("[SCANNER] MAC randomized: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  newMac[0], newMac[1], newMac[2], newMac[3], newMac[4], newMac[5]);
}

void scanner_restore_mac() {
    if (macSaved) {
        esp_wifi_set_mac(WIFI_IF_STA, originalMac);
        Serial.println("[SCANNER] Original MAC restored");
    }
}
