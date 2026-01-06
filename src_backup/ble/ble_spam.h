/**
 * @file ble_spam.h
 * @brief BLE Spam - Get Schwifty Mode
 *
 * Bluetooth Low Energy advertisement spam for various targets
 */

#ifndef BLE_SPAM_H
#define BLE_SPAM_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "../config.h"

// =============================================================================
// SPAM PAYLOADS
// =============================================================================

// Apple AirPods/AirDrop spam payloads
static const uint8_t APPLE_AIRPODS_PRO[] = {0x07, 0x19, 0x07, 0x0e, 0x20, 0x75, 0xaa, 0x30};
static const uint8_t APPLE_AIRDROP[] = {0x05, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Google Fast Pair model IDs
static const uint32_t GOOGLE_FAST_PAIR_MODELS[] = {
    0x000000,   // Generic
    0x00000A,   // Pixel Buds
    0x000011,   // Sony WH-1000XM4
    0x00002A,   // JBL Tune
    0x000041,   // Bose QC
};

// Samsung SmartTag payloads
static const uint8_t SAMSUNG_TAG[] = {0xFF, 0x00, 0xFD};

// Windows Swift Pair
static const uint8_t WINDOWS_PAIR[] = {0x06, 0xFF, 0x06, 0x00, 0x03};

// =============================================================================
// SPAM STATE
// =============================================================================
typedef struct {
    ble_target_t target;
    bool isSpamming;
    uint32_t spamCount;
    uint32_t lastSpamTime;
    uint16_t spamInterval;
    uint8_t currentPayload;
    bool randomize;
} ble_spam_state_t;

// =============================================================================
// BLE SPAM FUNCTIONS
// =============================================================================

/**
 * Initialize BLE spam module
 */
bool ble_spam_init(ble_spam_state_t* state);

/**
 * Start spamming specific target
 */
void ble_spam_start(ble_spam_state_t* state, ble_target_t target);

/**
 * Stop spamming
 */
void ble_spam_stop(ble_spam_state_t* state);

/**
 * Set spam interval
 */
void ble_spam_set_interval(ble_spam_state_t* state, uint16_t ms);

/**
 * Enable/disable payload randomization
 */
void ble_spam_set_randomize(ble_spam_state_t* state, bool enabled);

/**
 * Spam tick - call in loop
 */
void ble_spam_tick(ble_spam_state_t* state);

/**
 * Get spam count
 */
uint32_t ble_spam_get_count(ble_spam_state_t* state);

// =============================================================================
// TARGET-SPECIFIC SPAM
// =============================================================================

/**
 * Send Apple spam (AirPods popup)
 */
void ble_spam_apple(ble_spam_state_t* state);

/**
 * Send Google Fast Pair spam
 */
void ble_spam_android(ble_spam_state_t* state);

/**
 * Send Samsung SmartTag spam
 */
void ble_spam_samsung(ble_spam_state_t* state);

/**
 * Send Windows Swift Pair spam
 */
void ble_spam_windows(ble_spam_state_t* state);

/**
 * Send random spam to all targets
 */
void ble_spam_all(ble_spam_state_t* state);

// =============================================================================
// HELPERS
// =============================================================================

/**
 * Generate random advertisement payload
 */
void ble_generate_random_payload(uint8_t* payload, uint8_t len);

/**
 * Set BLE TX power
 */
void ble_set_power(int8_t power);

/**
 * Get BLE device name for spam
 */
const char* ble_get_random_name();

#endif // BLE_SPAM_H
