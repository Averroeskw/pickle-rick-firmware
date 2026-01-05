/**
 * @file main.cpp
 * @brief Pickle Rick - WiFi Security Firmware for K257
 *
 * Inspired by M5PORKCHOP - Rebranded Rick & Morty Space Theme
 * "I turned myself into a WiFi security tool, Morty!"
 *
 * SCROLL NAVIGATION: Use rotary encoder to switch between modes
 *
 * Copyright (c) 2025 AVERROES Tech Manufacturing
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
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
#include "wifi/wifi_scanner.h"
#include "modes/mode_manager.h"

// =============================================================================
// GLOBAL STATE
// =============================================================================
static app_state_t currentState = STATE_BOOT;
static rick_avatar_t rick;
static mode_manager_t modeManager;
static scanner_state_t wifiScanner;
static xp_stats_t xpStats;

// Hardware status
static bool sdMounted = false;
static bool gpsReady = false;
static bool loraReady = false;
static bool bleReady = false;

// GPS instance
TinyGPSPlus gps;

// Display buffer for status
static char statusLine[64];
static char modeLine[64];

// =============================================================================
// BOOT SPLASH - RICK IN BRAILLE
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
    Serial.println("  [SCROLL] Rotary to navigate | [PRESS] Enter mode | [ESC] Back");
    Serial.println();
}

// =============================================================================
// HARDWARE INITIALIZATION
// =============================================================================
bool initHardware() {
    Serial.println("[INIT] Initializing K257 hardware...");

    if (!instance.begin()) {
        Serial.println("[INIT] ❌ Hardware init failed!");
        return false;
    }

    Serial.println("[INIT] ✅ LilyGoLib initialized");

    // Configure display
    instance.setBrightness(DISP_BRIGHTNESS);
    instance.setRotation(DISP_ROTATION);

    // Enable keyboard with haptic
    instance.attachKeyboardFeedback(HAPTIC_ENABLED, HAPTIC_DURATION_MS);

    Serial.println("[INIT] ✅ Display & Input ready");
    return true;
}

bool initSDCard() {
    Serial.println("[INIT] Mounting SD card...");

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("[INIT] ⚠️ SD card not found");
        return false;
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

    return true;
}

bool initGPS() {
    #if GPS_ENABLED
    Serial.println("[INIT] Initializing GPS...");
    Serial1.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    gpsReady = true;
    Serial.println("[INIT] ✅ GPS UART ready");
    return true;
    #else
    return false;
    #endif
}

bool initLoRa() {
    #if LORA_ENABLED
    Serial.println("[INIT] Initializing LoRa SX1262...");
    // TODO: Initialize RadioLib
    loraReady = true;
    Serial.println("[INIT] ✅ LoRa ready (915 MHz)");
    return true;
    #else
    return false;
    #endif
}

bool initBLE() {
    #if BLE_ENABLED
    Serial.println("[INIT] Initializing BLE (NimBLE)...");
    NimBLEDevice::init("PICKLE_RICK");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    bleReady = true;
    Serial.println("[INIT] ✅ BLE ready");
    return true;
    #else
    return false;
    #endif
}

bool initWiFiScanner() {
    Serial.println("[INIT] Initializing WiFi scanner...");
    if (!scanner_init(&wifiScanner, 200)) {
        Serial.println("[INIT] ❌ WiFi scanner init failed!");
        return false;
    }
    Serial.println("[INIT] ✅ WiFi scanner ready (200 network capacity)");
    return true;
}

// =============================================================================
// INPUT HANDLING - SCROLL NAVIGATION
// =============================================================================
void handleInput() {
    // =========================================
    // ROTARY ENCODER - SCROLL BETWEEN MODES
    // =========================================
    RotaryMsg_t msg = instance.getRotary();

    if (msg.dir != ROTARY_DIR_NONE) {
        if (msg.dir == ROTARY_DIR_UP) {
            // Scroll UP = Previous mode
            mode_scroll(&modeManager, -1);
            instance.vibrator();  // Haptic feedback
        } else if (msg.dir == ROTARY_DIR_DOWN) {
            // Scroll DOWN = Next mode
            mode_scroll(&modeManager, 1);
            instance.vibrator();  // Haptic feedback
        }
        instance.clearRotaryMsg();

        // Print current selection
        if (modeManager.inMenu) {
            Serial.printf("\n[MENU] >>> %s %s <<<\n",
                          MODE_INFO[modeManager.menuIndex].icon,
                          MODE_INFO[modeManager.menuIndex].name);
            Serial.printf("[MENU]     %s\n", MODE_INFO[modeManager.menuIndex].description);
        }
    }

    // Rotary button press = SELECT
    if (msg.centerBtnPressed) {
        mode_select(&modeManager);
        instance.vibrator();
        instance.clearRotaryMsg();
    }

    // =========================================
    // KEYBOARD INPUT
    // =========================================
    char key;
    if (instance.getKeyChar(&key) > 0) {
        switch (key) {
            case KEY_MENU_ESC:  // ESC - Back to menu
                mode_back(&modeManager);
                instance.vibrator();
                break;

            case KEY_SELECT_ENTER:  // ENTER - Select
                mode_select(&modeManager);
                instance.vibrator();
                break;

            case KEY_ACTION_SPACE:  // SPACE - Mode action
                // Mode-specific action
                if (modeManager.currentMode == MODE_PORTAL && !wifiScanner.isScanning) {
                    scanner_start(&wifiScanner);
                } else if (modeManager.currentMode == MODE_PORTAL && wifiScanner.isScanning) {
                    scanner_stop(&wifiScanner);
                }
                break;

            case 'r':  // R - Randomize MAC
            case 'R':
                scanner_randomize_mac();
                break;

            case 'c':  // C - Clear networks
            case 'C':
                scanner_clear(&wifiScanner);
                break;
        }
    }
}

// =============================================================================
// MODE EXECUTION
// =============================================================================
void runCurrentMode() {
    switch (modeManager.currentMode) {
        case MODE_MENU:
            // Main menu - show current selection
            break;

        case MODE_PORTAL:
            // WiFi scanning - Portal Gun mode
            scanner_tick(&wifiScanner);
            break;

        case MODE_INTERDIMENSIONAL:
            // Handshake capture
            // TODO: handshake_tick()
            break;

        case MODE_SCHWIFTY:
            // BLE spam
            // TODO: ble_spam_tick()
            break;

        case MODE_WUBBA_LUBBA:
            // Wardriving
            // TODO: wardrive_tick()
            break;

        case MODE_SPECTRUM:
            // Spectrum analyzer
            // TODO: spectrum_tick()
            break;

        case MODE_LORA_MESH:
            // LoRa mesh
            // TODO: lora_tick()
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
void awardXP(uint32_t amount, const char* reason) {
    float modifier = rick_get_xp_multiplier(&rick);
    uint32_t awarded = (uint32_t)(amount * modifier);

    xpStats.totalXP += awarded;
    rick.xp = xpStats.totalXP;

    // Check for rank up
    rick_rank_t newRank = rick_calculate_rank(xpStats.totalXP);
    if (newRank > rick.rank) {
        rick.rank = newRank;
        Serial.printf("\n[RANK UP] 🎉 %s %s!\n",
                      rick_rank_icon(newRank),
                      rick_rank_name(newRank));
        instance.vibrator();
        delay(100);
        instance.vibrator();
    }

    Serial.printf("[XP] +%d (%s) | Total: %d | Rank: %s\n",
                  awarded, reason, xpStats.totalXP, rick_rank_name(rick.rank));
}

// =============================================================================
// STATUS DISPLAY
// =============================================================================
void showStatus() {
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus < 2000) return;
    lastStatus = millis();

    Serial.println();
    Serial.println("╔══════════════════════════════════════════════════════════════╗");

    // Current mode
    snprintf(modeLine, sizeof(modeLine), "║ %s %-20s                            ║",
             MODE_INFO[modeManager.currentMode].icon,
             MODE_INFO[modeManager.currentMode].name);
    Serial.println(modeLine);

    Serial.println("╠══════════════════════════════════════════════════════════════╣");

    // Status line
    snprintf(statusLine, sizeof(statusLine),
             "║ XP: %-6d | Rank: %-12s | Mood: %-8s        ║",
             xpStats.totalXP,
             rick_rank_name(rick.rank),
             rick.mood == MOOD_DRUNK ? "Drunk" :
             rick.mood == MOOD_GENIUS ? "Genius" :
             rick.mood == MOOD_ANGRY ? "Angry" : "Normal");
    Serial.println(statusLine);

    // Hardware status
    Serial.printf("║ SD: %s | GPS: %s | LoRa: %s | BLE: %s                   ║\n",
                  sdMounted ? "✅" : "❌",
                  gpsReady ? "✅" : "❌",
                  loraReady ? "✅" : "❌",
                  bleReady ? "✅" : "❌");

    // Mode-specific status
    if (modeManager.currentMode == MODE_PORTAL) {
        Serial.printf("║ Networks: %-4d | Channel: %-2d | Scanning: %-3s              ║\n",
                      wifiScanner.count,
                      wifiScanner.currentChannel,
                      wifiScanner.isScanning ? "YES" : "NO");
    }

    Serial.println("╚══════════════════════════════════════════════════════════════╝");

    // Menu hint when in menu
    if (modeManager.inMenu) {
        Serial.println("\n[SCROLL to select] [PRESS to enter] [ESC to back]");
        Serial.printf("\n>>> %s %s - %s\n",
                      MODE_INFO[modeManager.menuIndex].icon,
                      MODE_INFO[modeManager.menuIndex].name,
                      MODE_INFO[modeManager.menuIndex].description);
    }
}

// =============================================================================
// PRINT MENU
// =============================================================================
void printMenu() {
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║          🏠 RICK'S GARAGE              ║");
    Serial.println("╠════════════════════════════════════════╣");

    for (int i = 1; i < MODE_COUNT; i++) {
        if (i == modeManager.menuIndex) {
            Serial.printf("║ >> %s %-20s <<      ║\n",
                          MODE_INFO[i].icon, MODE_INFO[i].name);
        } else {
            Serial.printf("║    %s %-20s         ║\n",
                          MODE_INFO[i].icon, MODE_INFO[i].name);
        }
    }

    Serial.println("╚════════════════════════════════════════╝");
    Serial.printf("\nSelected: %s - %s\n",
                  MODE_INFO[modeManager.menuIndex].name,
                  MODE_INFO[modeManager.menuIndex].description);
}

// =============================================================================
// SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    showBootSplash();

    Serial.println("[BOOT] Starting Pickle Rick firmware...\n");

    // Initialize hardware
    if (!initHardware()) {
        Serial.println("[BOOT] ❌ Boot failed - hardware error");
        currentState = STATE_ERROR;
        return;
    }

    // Initialize subsystems
    initSDCard();
    initGPS();
    initLoRa();
    initBLE();
    initWiFiScanner();

    // Initialize Rick avatar
    rick_init(&rick);

    // Initialize XP
    xp_init(&xpStats);

    // Initialize mode manager
    mode_init(&modeManager);

    // Set initial state
    currentState = STATE_MENU;

    // Award first boot XP
    if (xpStats.totalXP == 0) {
        awardXP(100, "First Boot - Wubba Lubba Dub Dub!");
    }

    Serial.println("\n[BOOT] ✅ Pickle Rick ready!");
    Serial.println("[BOOT] \"Nobody exists on purpose. Nobody belongs anywhere.");
    Serial.println("        Everybody's gonna die. Come wardrive with me.\"\n");

    // Print initial menu
    printMenu();

    // Haptic feedback for boot complete
    instance.vibrator();
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
    // Run LilyGo event loop
    instance.loop();

    // Handle inputs (scroll navigation)
    handleInput();

    // Update GPS
    updateGPS();

    // Update Rick avatar
    rick_update_animation(&rick);

    // Run current mode
    runCurrentMode();

    // Show status periodically
    showStatus();

    // Small delay
    delay(10);
}
