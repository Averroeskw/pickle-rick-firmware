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
#include <Wire.h>
#include <Adafruit_TCA8418.h>

// Local includes
#include "config.h"
#include "core/pickle_rick.h"
#include "core/xp_system.h"
#include "wifi/wifi_scanner.h"
#include "modes/mode_manager.h"
#include "ui/ui_manager.h"
#include "ui/ui_theme.h"

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
static bool kbReady = false;
static bool uiReady = false;

// UI state
static ui_state_t uiState;

// GPS instance
TinyGPSPlus gps;

// Keyboard controller
Adafruit_TCA8418 keyboard;

// Rotary encoder state
static volatile int rotaryPosition = 0;
static volatile bool rotaryPressed = false;
static int lastRotaryA = HIGH;

// Display buffer for status
static char statusLine[64];
static char modeLine[64];

// Forward declarations
void printMenu();

// =============================================================================
// ROTARY ENCODER ISR
// =============================================================================
void IRAM_ATTR rotaryISR() {
    int a = digitalRead(ROTARY_A_PIN);
    int b = digitalRead(ROTARY_B_PIN);

    if (a != lastRotaryA) {
        if (b != a) {
            rotaryPosition++;
        } else {
            rotaryPosition--;
        }
        lastRotaryA = a;
    }
}

void IRAM_ATTR rotaryButtonISR() {
    rotaryPressed = true;
}

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
bool initI2C() {
    Serial.println("[INIT] Initializing I2C bus...");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
    Serial.println("[INIT] ✅ I2C ready");
    return true;
}

bool initKeyboard() {
    Serial.println("[INIT] Initializing keyboard (TCA8418)...");

    if (!keyboard.begin(TCA8418_I2C_ADDR, &Wire)) {
        Serial.println("[INIT] ⚠️ Keyboard not found");
        return false;
    }

    // Configure keyboard matrix (adjust for actual layout)
    keyboard.matrix(7, 7);  // 7x7 matrix
    keyboard.flush();

    kbReady = true;
    Serial.println("[INIT] ✅ Keyboard ready");
    return true;
}

bool initRotaryEncoder() {
    Serial.println("[INIT] Initializing rotary encoder...");

    pinMode(ROTARY_A_PIN, INPUT_PULLUP);
    pinMode(ROTARY_B_PIN, INPUT_PULLUP);
    pinMode(ROTARY_BTN_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ROTARY_A_PIN), rotaryISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ROTARY_BTN_PIN), rotaryButtonISR, FALLING);

    Serial.println("[INIT] ✅ Rotary encoder ready");
    return true;
}

bool initSDCard() {
    Serial.println("[INIT] Mounting SD card...");

    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);

    if (!SD.begin(SD_CS_PIN, SPI)) {
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
    // TODO: Initialize RadioLib with SX1262
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

bool initUI() {
    Serial.println("[INIT] Initializing LVGL UI...");
    if (!ui_init(&uiState, &rick)) {
        Serial.println("[INIT] ❌ UI init failed!");
        return false;
    }
    uiReady = true;
    Serial.println("[INIT] ✅ LVGL UI ready");
    return true;
}

// =============================================================================
// INPUT HANDLING - SCROLL NAVIGATION
// =============================================================================
void handleInput() {
    static int lastPosition = 0;

    // =========================================
    // ROTARY ENCODER - SCROLL BETWEEN MODES
    // =========================================
    int delta = rotaryPosition - lastPosition;
    if (delta != 0) {
        if (delta > 0) {
            // Scroll DOWN = Next mode
            mode_scroll(&modeManager, 1);
        } else {
            // Scroll UP = Previous mode
            mode_scroll(&modeManager, -1);
        }
        lastPosition = rotaryPosition;

        // Print current selection
        if (modeManager.inMenu) {
            Serial.printf("\n[MENU] >>> %s %s <<<\n",
                          MODE_INFO[modeManager.menuIndex].icon,
                          MODE_INFO[modeManager.menuIndex].name);
            Serial.printf("[MENU]     %s\n", MODE_INFO[modeManager.menuIndex].description);
        }
    }

    // Rotary button press = SELECT
    if (rotaryPressed) {
        mode_select(&modeManager);
        rotaryPressed = false;
    }

    // =========================================
    // KEYBOARD INPUT
    // =========================================
    if (kbReady && keyboard.available() > 0) {
        uint8_t k = keyboard.getEvent();
        if (k & 0x80) {  // Key press (not release)
            uint8_t key = k & 0x7F;

            switch (key) {
                case 0x1B:  // ESC - Back to menu
                    mode_back(&modeManager);
                    break;

                case 0x0D:  // ENTER - Select
                    mode_select(&modeManager);
                    break;

                case ' ':  // SPACE - Mode action
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

    // Serial keyboard fallback
    if (Serial.available() > 0) {
        char c = Serial.read();
        switch (c) {
            case 'n': case 'N':  // Next mode
                mode_scroll(&modeManager, 1);
                break;
            case 'p': case 'P':  // Prev mode
                mode_scroll(&modeManager, -1);
                break;
            case '\r': case '\n':  // Enter
                mode_select(&modeManager);
                break;
            case 'b': case 'B': case 27:  // Back/ESC
                mode_back(&modeManager);
                break;
            case ' ':  // Space - action
                if (modeManager.currentMode == MODE_PORTAL) {
                    if (!wifiScanner.isScanning) scanner_start(&wifiScanner);
                    else scanner_stop(&wifiScanner);
                }
                break;
            case 'r': case 'R':
                scanner_randomize_mac();
                break;
            case 'm': case 'M':
                printMenu();
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
            break;

        case MODE_PORTAL:
            scanner_tick(&wifiScanner);
            break;

        case MODE_INTERDIMENSIONAL:
            // TODO: handshake_tick()
            break;

        case MODE_SCHWIFTY:
            // TODO: ble_spam_tick()
            break;

        case MODE_WUBBA_LUBBA:
            // TODO: wardrive_tick()
            break;

        case MODE_SPECTRUM:
            // TODO: spectrum_tick()
            break;

        case MODE_LORA_MESH:
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

    snprintf(modeLine, sizeof(modeLine), "║ %s %-20s                            ║",
             MODE_INFO[modeManager.currentMode].icon,
             MODE_INFO[modeManager.currentMode].name);
    Serial.println(modeLine);

    Serial.println("╠══════════════════════════════════════════════════════════════╣");

    snprintf(statusLine, sizeof(statusLine),
             "║ XP: %-6d | Rank: %-12s | Mood: %-8s        ║",
             xpStats.totalXP,
             rick_rank_name(rick.rank),
             rick.mood == MOOD_DRUNK ? "Drunk" :
             rick.mood == MOOD_GENIUS ? "Genius" :
             rick.mood == MOOD_ANGRY ? "Angry" : "Normal");
    Serial.println(statusLine);

    Serial.printf("║ SD: %s | GPS: %s | LoRa: %s | BLE: %s | KB: %s              ║\n",
                  sdMounted ? "OK" : "--",
                  gpsReady ? "OK" : "--",
                  loraReady ? "OK" : "--",
                  bleReady ? "OK" : "--",
                  kbReady ? "OK" : "--");

    if (modeManager.currentMode == MODE_PORTAL) {
        Serial.printf("║ Networks: %-4d | Channel: %-2d | Scanning: %-3s              ║\n",
                      wifiScanner.count,
                      wifiScanner.currentChannel,
                      wifiScanner.isScanning ? "YES" : "NO");
    }

    Serial.println("╚══════════════════════════════════════════════════════════════╝");

    if (modeManager.inMenu) {
        Serial.println("\n[N/P] Navigate | [ENTER] Select | [B] Back | [M] Menu");
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
    Serial.println("║          RICK'S GARAGE                 ║");
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
    initI2C();
    initKeyboard();
    initRotaryEncoder();
    initSDCard();
    initGPS();
    initLoRa();
    initBLE();
    initWiFiScanner();
    initUI();  // Initialize LVGL display

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

    Serial.println("\n[BOOT] Pickle Rick ready!");
    Serial.println("[BOOT] \"Nobody exists on purpose. Come wardrive with me.\"\n");

    // Print initial menu
    printMenu();
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
    // Update LVGL (must be called frequently)
    if (uiReady) {
        ui_update(&uiState);
    }

    // Handle inputs (scroll navigation)
    handleInput();

    // Update GPS
    updateGPS();

    // Update Rick avatar
    rick_update_animation(&rick);

    // Run current mode
    runCurrentMode();

    // Show status periodically (serial only)
    showStatus();

    delay(5);  // Short delay for LVGL responsiveness
}
