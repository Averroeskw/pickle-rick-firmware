/**
 * @file wifi_scanner.h
 * @brief WiFi Scanner - Portal Gun Mode
 *
 * Passive and active WiFi scanning with channel hopping
 */

#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <Arduino.h>
#include <esp_wifi.h>
#include "../config.h"

// =============================================================================
// NETWORK DATA STRUCTURES
// =============================================================================
typedef struct {
    uint8_t bssid[6];
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    wifi_auth_mode_t authmode;
    bool hidden;
    uint32_t firstSeen;
    uint32_t lastSeen;
    float latitude;
    float longitude;
    bool hasHandshake;
    bool hasPMKID;
} network_info_t;

typedef struct {
    network_info_t* networks;
    uint16_t count;
    uint16_t capacity;
    uint8_t currentChannel;
    bool isScanning;
    bool isHopping;
    uint32_t scanStartTime;
    uint32_t lastHopTime;
} scanner_state_t;

// =============================================================================
// SCANNER FUNCTIONS
// =============================================================================

/**
 * Initialize WiFi scanner
 */
bool scanner_init(scanner_state_t* state, uint16_t max_networks);

/**
 * Start passive scanning
 */
void scanner_start(scanner_state_t* state);

/**
 * Stop scanning
 */
void scanner_stop(scanner_state_t* state);

/**
 * Set specific channel
 */
void scanner_set_channel(scanner_state_t* state, uint8_t channel);

/**
 * Enable/disable channel hopping
 */
void scanner_set_hopping(scanner_state_t* state, bool enabled);

/**
 * Scanner tick - call in loop
 */
void scanner_tick(scanner_state_t* state);

/**
 * Get network by index
 */
network_info_t* scanner_get_network(scanner_state_t* state, uint16_t index);

/**
 * Get network by BSSID
 */
network_info_t* scanner_find_bssid(scanner_state_t* state, const uint8_t* bssid);

/**
 * Add or update network
 */
void scanner_add_network(scanner_state_t* state, network_info_t* network);

/**
 * Clear all networks
 */
void scanner_clear(scanner_state_t* state);

/**
 * Export networks to WiGLE CSV format
 */
bool scanner_export_wigle(scanner_state_t* state, const char* filename);

/**
 * Get network count by auth type
 */
uint16_t scanner_count_by_auth(scanner_state_t* state, wifi_auth_mode_t auth);

/**
 * Get hidden network count
 */
uint16_t scanner_count_hidden(scanner_state_t* state);

// =============================================================================
// PROMISCUOUS MODE
// =============================================================================

/**
 * Enable promiscuous mode for packet capture
 */
void scanner_enable_promisc(scanner_state_t* state);

/**
 * Disable promiscuous mode
 */
void scanner_disable_promisc(scanner_state_t* state);

/**
 * Set promiscuous callback
 */
void scanner_set_callback(wifi_promiscuous_cb_t callback);

// =============================================================================
// MAC RANDOMIZATION
// =============================================================================

/**
 * Randomize MAC address
 */
void scanner_randomize_mac();

/**
 * Restore original MAC
 */
void scanner_restore_mac();

#endif // WIFI_SCANNER_H
