/**
 * @file main.cpp
 * @brief Pickle Rick - WiFi Security Firmware for K257
 *
 * Inspired by M5PORKCHOP - Rebranded Rick & Morty Space Theme
 * "I turned myself into a WiFi security tool, Morty!"
 *
 * Copyright (c) 2025 AVERROES Tech Manufacturing
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <NimBLEDevice.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>

// LilyGo library for K257 hardware
#include <LilyGoLib.h>

// Local includes
#include "config.h"
#include "core/pickle_rick.h"
#include "core/xp_system.h"
#include "core/sdlog.h"
#include "wifi/wifi_scanner.h"
#include "wifi/handshake_capture.h"
#include "wifi/deauth.h"
#include "ble/ble_spam.h"
#include "gps/wardriving.h"
#include "ui/ui_manager.h"
#include "modes/mode_manager.h"

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================
void initHardware();
void initDisplay();
void initSDCard();
void initGPS();
void initLoRa();
void initBLE();
void showBootSplash();
void updateDisplay();
void handleInput();
void runCurrentMode();

// =============================================================================
// GLOBAL STATE
// =============================================================================
static app_state_t currentState = STATE_BOOT;
static operation_mode_t currentMode = MODE_MENU;
static rick_mood_t rickMood = MOOD_DRUNK;  // Classic Rick
static bool sdMounted = false;
static bool gpsReady = false;
static bool loraReady = false;
static bool bleReady = false;

// GPS instance
TinyGPSPlus gps;

// XP and gamification
static uint32_t totalXP = 0;
static rick_rank_t currentRank = RANK_MORTY;

// Stats
static uint32_t networksFound = 0;
static uint32_t handshakesCaptured = 0;
static uint32_t pmkidsExtracted = 0;
static uint32_t bleSpamCount = 0;
static uint32_t loraMessagesSent = 0;

// =============================================================================
// BOOT SEQUENCE
// =============================================================================
void showBootSplash() {
    Serial.println();
    Serial.println("  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣤⣤⣤⣤⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀");
    Serial.println("  ⠀⠀⠀⠀⠀⠀⠀⣠⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⣄⠀⠀⠀⠀⠀⠀⠀");
    Serial.println("  ⠀⠀⠀⠀⠀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣄⠀⠀⠀⠀⠀");
    Serial.println("  ⠀⠀⠀⠀⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⠀⠀⠀⠀");
    Serial.println("  ⠀⠀⠀⣸⣿⣿⣿⣿⡿⠿⠿⠿⣿⣿⣿⣿⠿⠿⠿⢿⣿⣿⣿⣿⡇⠀⠀⠀");
    Serial.println("  ⠀⠀⢀⣿⣿⣿⣿⡏⣤⣤⣤⡄⠈⣿⣿⠁⢠⣤⣤⣤⢹⣿⣿⣿⣿⡀⠀⠀");
    Serial.println("  ⠀⠀⢸⣿⣿⣿⣿⡇⠛⠛⠛⠃⢠⣿⣿⡄⠘⠛⠛⠛⢸⣿⣿⣿⣿⡇⠀⠀");
    Serial.println("  ⠀⠀⢸⣿⣿⣿⣿⣧⠀⠀⠀⣠⣿⣿⣿⣿⣄⠀⠀⠀⣼⣿⣿⣿⣿⡇⠀⠀");
    Serial.println("  ⠀⠀⠘⣿⣿⣿⣿⣿⣷⣶⣾⣿⣿⣿⣿⣿⣿⣷⣶⣾⣿⣿⣿⣿⣿⠃⠀⠀");
    Serial.println("  ⠀⠀⠀⠹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⠀⠀⠀");
    Serial.println("  ⠀⠀⠀⠀⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠋⠀⠀⠀⠀");
    Serial.println("  ⠀⠀⠀⠀⠀⠀⠙⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠋⠀⠀⠀⠀⠀⠀");
    Serial.println("  ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠛⠻⠿⠿⠿⠿⠟⠛⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀");
    Serial.println();
    Serial.println("  ╔═══════════════════════════════════════════════════════════╗");
    Serial.println("  ║                     PICKLE RICK                           ║");
    Serial.println("  ║         WiFi Security Tool for K257 T-LoRa Pager          ║");
    Serial.println("  ╠═══════════════════════════════════════════════════════════╣");
    Serial.println("  ║   \"I turned myself into a WiFi tool, Morty!\"              ║");
    Serial.println("  ║                                                           ║");
    Serial.println("  ║   Version: " PICKLE_RICK_VERSION "            Codename: " PICKLE_RICK_CODENAME "   ║");
    Serial.println("  ║   AVERROES Tech Manufacturing                             ║");
    Serial.println("  ╚═══════════════════════════════════════════════════════════╝");
    Serial.println();
}

// =============================================================================
// HARDWARE INITIALIZATION
// =============================================================================
void initHardware() {
    Serial.println("[INIT] Initializing K257 hardware...");

    if (!instance.begin()) {
        Serial.println("[INIT] ❌ Hardware init failed!");
        currentState = STATE_ERROR;
        return;
    }

    Serial.println("[INIT] ✅ LilyGoLib initialized");

    // Configure display
    instance.setBrightness(DISP_BRIGHTNESS);
    instance.setRotation(DISP_ROTATION);

    // Enable keyboard with haptic
    instance.attachKeyboardFeedback(HAPTIC_ENABLED, HAPTIC_DURATION_MS);

    Serial.println("[INIT] ✅ Display & Input ready");
}

void initSDCard() {
    Serial.println("[INIT] Mounting SD card...");

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("[INIT] ⚠️ SD card not found");
        sdMounted = false;
        return;
    }

    sdMounted = true;
    Serial.printf("[INIT] ✅ SD card mounted (%.2f GB)\n",
                  SD.cardSize() / (1024.0 * 1024.0 * 1024.0));

    // Create directories
    SD.mkdir(DIR_ROOT);
    SD.mkdir(DIR_HANDSHAKES);
    SD.mkdir(DIR_PMKID);
    SD.mkdir(DIR_WARDRIVING);
    SD.mkdir(DIR_LOGS);
    SD.mkdir(DIR_CONFIG);
    SD.mkdir(DIR_XP);
    SD.mkdir(DIR_ACHIEVEMENTS);

    // Load saved XP
    if (XP_PERSIST_ENABLED) {
        // TODO: Load XP from SD
    }
}

void initGPS() {
    #if GPS_ENABLED
    Serial.println("[INIT] Initializing GPS...");

    // Enable GPS via expander
    // instance.expanderWrite(XL_GPS_EN, HIGH);

    Serial1.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    gpsReady = true;
    Serial.println("[INIT] ✅ GPS UART ready");
    #else
    Serial.println("[INIT] GPS disabled in config");
    #endif
}

void initLoRa() {
    #if LORA_ENABLED
    Serial.println("[INIT] Initializing LoRa SX1262...");

    // Enable LoRa via expander
    // instance.expanderWrite(XL_LORA_EN, HIGH);

    // TODO: Initialize RadioLib SX1262
    loraReady = true;
    Serial.println("[INIT] ✅ LoRa ready (915 MHz)");
    #else
    Serial.println("[INIT] LoRa disabled in config");
    #endif
}

void initBLE() {
    #if BLE_ENABLED
    Serial.println("[INIT] Initializing BLE (NimBLE)...");

    NimBLEDevice::init("PICKLE_RICK");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    bleReady = true;
    Serial.println("[INIT] ✅ BLE ready (Get Schwifty mode available)");
    #else
    Serial.println("[INIT] BLE disabled in config");
    #endif
}

// =============================================================================
// WIFI PROMISCUOUS MODE CALLBACK
// =============================================================================
typedef struct {
    unsigned frame_ctrl:16;
    unsigned duration_id:16;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    unsigned sequence_ctrl:16;
    uint8_t addr4[6];
} wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0];
} wifi_ieee80211_packet_t;

void IRAM_ATTR wifiSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) return;

    const wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)pkt->payload;
    const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

    // Check for EAPOL frames (handshake)
    if (type == WIFI_PKT_DATA) {
        // TODO: EAPOL detection and capture
    }

    // Check for beacon frames (PMKID in RSN IE)
    if (type == WIFI_PKT_MGMT) {
        uint16_t frame_type = hdr->frame_ctrl & 0x00FC;
        if (frame_type == 0x0080) {  // Beacon
            // TODO: PMKID extraction
        }
    }
}

// =============================================================================
// INPUT HANDLING
// =============================================================================
void handleInput() {
    // Keyboard input
    char key;
    if (instance.getKeyChar(&key) > 0) {
        Serial.printf("[INPUT] Key: 0x%02X ('%c')\n", key, key);

        switch (key) {
            case KEY_MENU_ESC:
                if (currentMode != MODE_MENU) {
                    currentMode = MODE_MENU;
                    currentState = STATE_MENU;
                }
                break;

            case KEY_SELECT_ENTER:
                // Select current menu item
                break;

            case KEY_ACTION_SPACE:
                // Mode-specific action
                break;
        }
    }

    // Rotary encoder
    RotaryMsg_t msg = instance.getRotary();
    if (msg.dir != ROTARY_DIR_NONE) {
        if (msg.dir == ROTARY_DIR_UP) {
            // Navigate up/next
        } else if (msg.dir == ROTARY_DIR_DOWN) {
            // Navigate down/prev
        }
        instance.clearRotaryMsg();
    }

    if (msg.centerBtnPressed) {
        // Select/confirm
        instance.clearRotaryMsg();
    }
}

// =============================================================================
// MODE EXECUTION
// =============================================================================
void runCurrentMode() {
    switch (currentMode) {
        case MODE_MENU:
            // Main menu - Rick's Garage
            break;

        case MODE_PORTAL:
            // WiFi scanning - Portal Gun mode
            // wifi_scanner_tick();
            break;

        case MODE_INTERDIMENSIONAL:
            // Handshake capture - Interdimensional Cable
            // handshake_capture_tick();
            break;

        case MODE_SCHWIFTY:
            // BLE spam - Get Schwifty
            // ble_spam_tick();
            break;

        case MODE_WUBBA_LUBBA:
            // Wardriving - Wubba Lubba Dub Dub
            // wardriving_tick();
            break;

        case MODE_CHILL:
            // Passive mode - Chill with Unity
            break;

        case MODE_SPECTRUM:
            // Spectrum analyzer - Microverse
            // spectrum_tick();
            break;

        case MODE_LORA_MESH:
            // LoRa mesh - Council of Ricks
            // lora_mesh_tick();
            break;

        case MODE_PLUMBUS:
            // File manager - Plumbus Commander
            // file_manager_tick();
            break;

        case MODE_SETTINGS:
            // Settings - Garage Workshop
            break;

        default:
            break;
    }
}

// =============================================================================
// GPS UPDATE
// =============================================================================
void updateGPS() {
    #if GPS_ENABLED
    while (Serial1.available() > 0) {
        gps.encode(Serial1.read());
    }
    #endif
}

// =============================================================================
// XP SYSTEM
// =============================================================================
void awardXP(uint32_t amount) {
    // Apply mood modifier
    float modifier = 1.0f;
    switch (rickMood) {
        case MOOD_GENIUS:      modifier = 1.2f; break;
        case MOOD_PICKLE:      modifier = 1.5f; break;
        case MOOD_DEPRESSED:   modifier = 0.9f; break;
        default:               modifier = 1.0f; break;
    }

    totalXP += (uint32_t)(amount * modifier);

    // Check for rank up
    // TODO: Rank calculation

    Serial.printf("[XP] +%d (Total: %d, Rank: %d)\n",
                  (int)(amount * modifier), totalXP, currentRank);
}

// =============================================================================
// DISPLAY UPDATE
// =============================================================================
void updateDisplay() {
    // TODO: LVGL UI update
    // For now, just log state
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
        lastLog = millis();
        Serial.printf("[STATUS] Mode: %s | XP: %d | Rank: %d | GPS: %s | LoRa: %s\n",
                      MODE_NAMES[currentMode],
                      totalXP,
                      currentRank,
                      gpsReady ? "OK" : "OFF",
                      loraReady ? "OK" : "OFF");
    }
}

// =============================================================================
// SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    showBootSplash();

    Serial.println("[BOOT] Starting Pickle Rick firmware...");
    Serial.println();

    // Initialize hardware
    initHardware();
    if (currentState == STATE_ERROR) {
        Serial.println("[BOOT] ❌ Boot failed - hardware error");
        return;
    }

    initSDCard();
    initGPS();
    initLoRa();
    initBLE();

    // Set initial state
    currentState = STATE_MENU;
    currentMode = MODE_MENU;
    rickMood = MOOD_DRUNK;  // Classic Rick

    // Award first boot XP
    if (totalXP == 0) {
        awardXP(100);  // "Wubba Lubba Dub Dub" achievement
        Serial.println("[ACHIEVEMENT] 🏆 Wubba Lubba Dub Dub - First Boot!");
    }

    Serial.println();
    Serial.println("[BOOT] ✅ Pickle Rick ready!");
    Serial.println("[BOOT] \"Nobody exists on purpose. Nobody belongs anywhere.");
    Serial.println("        Everybody's gonna die. Come wardrive with me.\"");
    Serial.println();

    // Haptic feedback for boot complete
    instance.vibrator();
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
    // Run LilyGo event loop
    instance.loop();

    // Handle inputs
    handleInput();

    // Update GPS
    updateGPS();

    // Run current mode
    runCurrentMode();

    // Update display (LVGL)
    updateDisplay();

    // Small delay to prevent busy-looping
    delay(10);
}
