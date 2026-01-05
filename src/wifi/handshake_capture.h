/**
 * @file handshake_capture.h
 * @brief Handshake & PMKID Capture - Interdimensional Cable Mode
 *
 * WPA/WPA2 4-way handshake capture and PMKID extraction
 */

#ifndef HANDSHAKE_CAPTURE_H
#define HANDSHAKE_CAPTURE_H

#include <Arduino.h>
#include "../config.h"

// =============================================================================
// EAPOL FRAME STRUCTURES
// =============================================================================
typedef struct {
    uint8_t version;
    uint8_t type;
    uint16_t length;
    uint8_t key_descriptor;
    uint16_t key_info;
    uint16_t key_length;
    uint8_t replay_counter[8];
    uint8_t key_nonce[32];
    uint8_t key_iv[16];
    uint8_t key_rsc[8];
    uint8_t key_id[8];
    uint8_t key_mic[16];
    uint16_t key_data_length;
} __attribute__((packed)) eapol_key_t;

typedef struct {
    uint8_t bssid[6];
    uint8_t station[6];
    char ssid[33];
    uint8_t anonce[32];
    uint8_t snonce[32];
    uint8_t mic[16];
    uint8_t eapol[256];
    uint16_t eapolLen;
    uint8_t keyver;
    bool hasFrame1;
    bool hasFrame2;
    bool hasFrame3;
    bool hasFrame4;
    bool complete;
    uint32_t captureTime;
} handshake_t;

typedef struct {
    uint8_t bssid[6];
    char ssid[33];
    uint8_t pmkid[16];
    uint32_t captureTime;
} pmkid_t;

// =============================================================================
// CAPTURE STATE
// =============================================================================
typedef struct {
    handshake_t* handshakes;
    uint16_t handshakeCount;
    uint16_t handshakeCapacity;

    pmkid_t* pmkids;
    uint16_t pmkidCount;
    uint16_t pmkidCapacity;

    uint8_t targetBSSID[6];
    bool hasTarget;
    bool isCapturing;
    uint32_t captureStartTime;
} capture_state_t;

// =============================================================================
// CAPTURE FUNCTIONS
// =============================================================================

/**
 * Initialize capture system
 */
bool capture_init(capture_state_t* state, uint16_t max_handshakes, uint16_t max_pmkids);

/**
 * Start capturing for specific target
 */
void capture_start_target(capture_state_t* state, const uint8_t* bssid, const char* ssid);

/**
 * Start capturing on all networks
 */
void capture_start_all(capture_state_t* state);

/**
 * Stop capturing
 */
void capture_stop(capture_state_t* state);

/**
 * Process incoming EAPOL frame
 */
void capture_process_eapol(capture_state_t* state, const uint8_t* packet, uint16_t len);

/**
 * Process beacon for PMKID extraction
 */
void capture_process_beacon(capture_state_t* state, const uint8_t* packet, uint16_t len);

/**
 * Check if handshake is complete
 */
bool capture_is_complete(handshake_t* handshake);

/**
 * Save handshake to file (22000 hashcat format)
 */
bool capture_save_handshake(handshake_t* handshake, const char* filename);

/**
 * Save PMKID to file (22000 hashcat format)
 */
bool capture_save_pmkid(pmkid_t* pmkid, const char* filename);

/**
 * Save all captures to SD card
 */
bool capture_save_all(capture_state_t* state);

/**
 * Export to hccapx format
 */
bool capture_export_hccapx(handshake_t* handshake, const char* filename);

/**
 * Get handshake count
 */
uint16_t capture_handshake_count(capture_state_t* state);

/**
 * Get PMKID count
 */
uint16_t capture_pmkid_count(capture_state_t* state);

/**
 * Clear all captures
 */
void capture_clear(capture_state_t* state);

// =============================================================================
// DEAUTH FUNCTIONS
// =============================================================================

/**
 * Send deauthentication frame
 */
void deauth_send(const uint8_t* bssid, const uint8_t* station, uint8_t reason, uint8_t count);

/**
 * Send broadcast deauth
 */
void deauth_broadcast(const uint8_t* bssid, uint8_t reason, uint8_t count);

/**
 * Send deauth to all clients
 */
void deauth_all_clients(const uint8_t* bssid, uint8_t reason, uint8_t count);

#endif // HANDSHAKE_CAPTURE_H
