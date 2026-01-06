/**
 * RICK - WiFi Security Tool for K257 T-LoRa Pager
 * Rick & Morty Space Theme with LVGL 9
 * Created for an alter Dimension By Archie
 * "Wubba Lubba Dub Dub!"
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <NimBLEDevice.h>
#include <SD.h>
#include <TinyGPSPlus.h>
#include "config.h"

// =============================================================================
// HAPTIC FEEDBACK LEVELS
// =============================================================================
#define HAPTIC_LIGHT    1   // Light tap (keypress, rotation)
#define HAPTIC_MEDIUM   14  // Medium (short button press)
#define HAPTIC_STRONG   47  // Strong (long press, confirmations)

// =============================================================================
// STATE
// =============================================================================
static screen_t currentScreen = SCREEN_BOOT;
static int menuIndex = 0;
static uint32_t totalXP = 0;
static rick_rank_t currentRank = RANK_MORTY;
static bool kbBacklightOn = true;

// WiFi scanner state
static bool wifiScanning = false;
static uint16_t networkCount = 0;
static int8_t scanChannel = 1;
static uint32_t lastChannelHop = 0;
static uint32_t lastWifiScan = 0;

// BLE spam state
static bool bleSpamming = false;
static ble_target_t bleTarget = BLE_TARGET_ALL;
static uint32_t bleSpamCount = 0;
static uint32_t lastBleSpam = 0;

// GPS Wardriving state
static bool gpsActive = false;
static bool gpsFix = false;
static double gpsLat = 0, gpsLon = 0;
static uint32_t gpsNetworksLogged = 0;
static uint32_t lastGpsUpdate = 0;

// LoRa Mesh state
static bool loraActive = false;
static bool loraInitialized = false;
static uint32_t loraMsgSent = 0;
static uint32_t loraMsgRecv = 0;
static int16_t loraLastRssi = 0;
static char loraLastMsg[64] = "";

// Settings state
static uint8_t settingsBrightness = 12;
static uint8_t settingsLoraFreq = 0;  // 0=915MHz, 1=868MHz
static uint8_t settingsIndex = 0;
static bool settingsEditing = false;

// SD Card state
static bool sdCardReady = false;
static uint32_t sdTotalMB = 0;
static uint32_t sdUsedMB = 0;
static int fileCount = 0;

// =============================================================================
// COLORS (Rick & Morty Theme)
// =============================================================================
static lv_color_t colBlack, colWhite, colGreen, colCyan, colYellow, colGray, colDim, colRed, colOrange;

static void initColors() {
    colBlack = lv_color_black();
    colWhite = lv_color_white();
    colGreen = lv_color_make(0, 255, 0);
    colCyan = lv_color_make(0, 206, 209);
    colYellow = lv_color_make(255, 215, 0);
    colGray = lv_color_make(100, 100, 100);
    colDim = lv_color_make(40, 40, 40);
    colRed = lv_color_make(255, 68, 68);
    colOrange = lv_color_make(255, 140, 0);
}

// =============================================================================
// SCREENS & UI ELEMENTS
// =============================================================================
static lv_obj_t* scrBoot;
static lv_obj_t* scrMenu;
static lv_obj_t* scrPortal;      // WiFi Scanner
static lv_obj_t* scrSchwifty;    // BLE Spam
static lv_obj_t* scrWubba;       // GPS Wardriving
static lv_obj_t* scrCouncil;     // LoRa Mesh
static lv_obj_t* scrPlumbus;     // File Manager
static lv_obj_t* scrSettings;    // Settings

// Status bar labels (per screen)
static lv_obj_t* lblBattery;
static lv_obj_t* lblTime;

// Menu labels
static lv_obj_t* lblMenuItems[MENU_ITEM_COUNT];
static lv_obj_t* lblMenuDesc;
static lv_obj_t* lblXpBar;
static lv_obj_t* lblRank;

// Portal screen (WiFi Scanner)
static lv_obj_t* lblPortalStatus;
static lv_obj_t* lblPortalNetworks[8];
static lv_obj_t* lblPortalCount;

// Schwifty screen (BLE Spam)
static lv_obj_t* lblSchwiftyStatus;
static lv_obj_t* lblSchwiftyCount;
static lv_obj_t* lblSchwiftyTarget;

// Wubba screen (GPS Wardriving)
static lv_obj_t* lblWubbaStatus;
static lv_obj_t* lblWubbaGps;
static lv_obj_t* lblWubbaCoords;
static lv_obj_t* lblWubbaLogged;

// Council screen (LoRa Mesh)
static lv_obj_t* lblCouncilStatus;
static lv_obj_t* lblCouncilStats;
static lv_obj_t* lblCouncilRssi;
static lv_obj_t* lblCouncilMsg;

// Plumbus screen (File Manager)
static lv_obj_t* lblPlumbusStatus;
static lv_obj_t* lblPlumbusSD;
static lv_obj_t* lblPlumbusFiles[6];

// Settings screen
static lv_obj_t* lblSettingsItems[5];

// =============================================================================
// STATUS BAR CREATION
// =============================================================================
lv_obj_t* createStatusBar(lv_obj_t* parent, const char* title) {
    lv_obj_t* bar = lv_obj_create(parent);
    lv_obj_set_size(bar, DISP_W, UI_BAR_H);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_style_bg_color(bar, colBlack, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);

    // Title (left)
    lv_obj_t* lblTitle = lv_label_create(bar);
    lv_obj_set_pos(lblTitle, 8, 4);
    lv_obj_set_style_text_color(lblTitle, colGreen, 0);
    lv_label_set_text(lblTitle, title);

    // Battery (right)
    lblBattery = lv_label_create(bar);
    lv_obj_set_pos(lblBattery, DISP_W - 50, 4);
    lv_obj_set_style_text_color(lblBattery, colCyan, 0);
    lv_label_set_text(lblBattery, "---");

    // Time (center-right)
    lblTime = lv_label_create(bar);
    lv_obj_set_pos(lblTime, DISP_W - 110, 4);
    lv_obj_set_style_text_color(lblTime, colWhite, 0);
    lv_label_set_text(lblTime, "--:--");

    // Divider line
    lv_obj_t* div = lv_obj_create(parent);
    lv_obj_set_size(div, DISP_W, 1);
    lv_obj_set_pos(div, 0, UI_BAR_H);
    lv_obj_set_style_bg_color(div, colGreen, 0);
    lv_obj_set_style_border_width(div, 0, 0);
    lv_obj_set_style_radius(div, 0, 0);

    return bar;
}

// =============================================================================
// BOOT SCREEN
// =============================================================================
void createBootScreen() {
    scrBoot = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scrBoot, colBlack, 0);

    lv_obj_t* lblTitle = lv_label_create(scrBoot);
    lv_obj_set_style_text_color(lblTitle, colGreen, 0);
    lv_obj_set_style_text_font(lblTitle, &lv_font_montserrat_48, 0);
    lv_label_set_text(lblTitle, "RICK");
    lv_obj_align(lblTitle, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t* lblSub = lv_label_create(scrBoot);
    lv_obj_set_style_text_color(lblSub, colCyan, 0);
    lv_obj_set_style_text_font(lblSub, &lv_font_montserrat_20, 0);
    lv_label_set_text(lblSub, "Wubba Lubba Dub Dub!");
    lv_obj_align(lblSub, LV_ALIGN_CENTER, 0, 20);

    lv_obj_t* lblVer = lv_label_create(scrBoot);
    lv_obj_set_style_text_color(lblVer, colGray, 0);
    lv_label_set_text_fmt(lblVer, "v%s - K257", RICK_VERSION);
    lv_obj_align(lblVer, LV_ALIGN_BOTTOM_MID, 0, -10);
}

// =============================================================================
// MENU SCREEN (Rick's Garage)
// =============================================================================
void createMenuScreen() {
    scrMenu = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scrMenu, colBlack, 0);
    createStatusBar(scrMenu, "RICK");

    lv_obj_t* lblTitle = lv_label_create(scrMenu);
    lv_obj_set_pos(lblTitle, 16, UI_MAIN_Y + 4);
    lv_obj_set_style_text_color(lblTitle, colCyan, 0);
    lv_obj_set_style_text_font(lblTitle, &lv_font_montserrat_20, 0);
    lv_label_set_text(lblTitle, "Rick's Garage");

    // Rank display (right side)
    lblRank = lv_label_create(scrMenu);
    lv_obj_set_pos(lblRank, DISP_W - 120, UI_MAIN_Y + 8);
    lv_obj_set_style_text_color(lblRank, colYellow, 0);
    lv_label_set_text(lblRank, "Morty");

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        lblMenuItems[i] = lv_label_create(scrMenu);
        lv_obj_set_pos(lblMenuItems[i], 24, UI_MAIN_Y + 32 + i * 22);
        lv_obj_set_style_text_color(lblMenuItems[i], colWhite, 0);
        lv_label_set_text_fmt(lblMenuItems[i], "%s %s", MENU_ITEMS[i].icon, MENU_ITEMS[i].name);
    }

    // XP Bar
    lblXpBar = lv_label_create(scrMenu);
    lv_obj_set_pos(lblXpBar, 200, UI_MAIN_Y + 32);
    lv_obj_set_style_text_color(lblXpBar, colGreen, 0);
    lv_label_set_text(lblXpBar, "XP: 0");

    lblMenuDesc = lv_label_create(scrMenu);
    lv_obj_set_pos(lblMenuDesc, 16, UI_BOT_Y - 20);
    lv_obj_set_style_text_color(lblMenuDesc, colGray, 0);
    lv_label_set_text(lblMenuDesc, MENU_ITEMS[0].desc);

    lv_obj_t* lblHint = lv_label_create(scrMenu);
    lv_obj_set_pos(lblHint, 16, UI_BOT_Y + 4);
    lv_obj_set_style_text_color(lblHint, colGray, 0);
    lv_label_set_text(lblHint, "Keys: P S W C F | Rotate/Enter: Select");
}

// =============================================================================
// PORTAL GUN SCREEN (WiFi Scanner)
// =============================================================================
void createPortalScreen() {
    scrPortal = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scrPortal, colBlack, 0);
    createStatusBar(scrPortal, "[P] Portal Gun");

    lblPortalStatus = lv_label_create(scrPortal);
    lv_obj_set_pos(lblPortalStatus, DISP_W - 100, UI_MAIN_Y + 4);
    lv_obj_set_style_text_color(lblPortalStatus, colYellow, 0);
    lv_label_set_text(lblPortalStatus, "IDLE");

    lblPortalCount = lv_label_create(scrPortal);
    lv_obj_set_pos(lblPortalCount, 16, UI_MAIN_Y + 4);
    lv_obj_set_style_text_color(lblPortalCount, colCyan, 0);
    lv_label_set_text(lblPortalCount, "Networks: 0 | Ch: 1");

    for (int i = 0; i < 8; i++) {
        lblPortalNetworks[i] = lv_label_create(scrPortal);
        lv_obj_set_pos(lblPortalNetworks[i], 16, UI_MAIN_Y + 28 + i * 18);
        lv_obj_set_style_text_color(lblPortalNetworks[i], colWhite, 0);
        lv_label_set_text(lblPortalNetworks[i], "");
    }

    lv_obj_t* lblHint = lv_label_create(scrPortal);
    lv_obj_set_pos(lblHint, 16, UI_BOT_Y + 4);
    lv_obj_set_style_text_color(lblHint, colGray, 0);
    lv_label_set_text(lblHint, "Space: Start/Stop | B/Long: Back | M: Menu");
}

// =============================================================================
// GET SCHWIFTY SCREEN (BLE Spam)
// =============================================================================
void createSchwiftyScreen() {
    scrSchwifty = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scrSchwifty, colBlack, 0);
    createStatusBar(scrSchwifty, "[S] Get Schwifty");

    lblSchwiftyStatus = lv_label_create(scrSchwifty);
    lv_obj_set_pos(lblSchwiftyStatus, DISP_W - 100, UI_MAIN_Y + 4);
    lv_obj_set_style_text_color(lblSchwiftyStatus, colGray, 0);
    lv_label_set_text(lblSchwiftyStatus, "STOPPED");

    lblSchwiftyTarget = lv_label_create(scrSchwifty);
    lv_obj_set_pos(lblSchwiftyTarget, 16, UI_MAIN_Y + 10);
    lv_obj_set_style_text_color(lblSchwiftyTarget, colCyan, 0);
    lv_obj_set_style_text_font(lblSchwiftyTarget, &lv_font_montserrat_20, 0);
    lv_label_set_text(lblSchwiftyTarget, "Target: ALL");

    lv_obj_t* lblCountLabel = lv_label_create(scrSchwifty);
    lv_obj_set_pos(lblCountLabel, 16, UI_MAIN_Y + 50);
    lv_obj_set_style_text_color(lblCountLabel, colGray, 0);
    lv_label_set_text(lblCountLabel, "Packets sent:");

    lblSchwiftyCount = lv_label_create(scrSchwifty);
    lv_obj_set_pos(lblSchwiftyCount, 16, UI_MAIN_Y + 75);
    lv_obj_set_style_text_color(lblSchwiftyCount, colGreen, 0);
    lv_obj_set_style_text_font(lblSchwiftyCount, &lv_font_montserrat_28, 0);
    lv_label_set_text(lblSchwiftyCount, "0");

    // Target list
    const char* targets[] = {"Apple", "Android", "Samsung", "Windows", "ALL"};
    for (int i = 0; i < 5; i++) {
        lv_obj_t* lbl = lv_label_create(scrSchwifty);
        lv_obj_set_pos(lbl, 200, UI_MAIN_Y + 10 + i * 20);
        lv_obj_set_style_text_color(lbl, colGray, 0);
        lv_label_set_text_fmt(lbl, "%s %s", i == 4 ? ">" : " ", targets[i]);
    }

    lv_obj_t* lblHint = lv_label_create(scrSchwifty);
    lv_obj_set_pos(lblHint, 16, UI_BOT_Y + 4);
    lv_obj_set_style_text_color(lblHint, colGray, 0);
    lv_label_set_text(lblHint, "Space: Start/Stop | 1-5: Target | B: Back");
}

// =============================================================================
// WUBBA LUBBA SCREEN (GPS Wardriving)
// =============================================================================
void createWubbaScreen() {
    scrWubba = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scrWubba, colBlack, 0);
    createStatusBar(scrWubba, "[W] Wubba Lubba");

    lblWubbaStatus = lv_label_create(scrWubba);
    lv_obj_set_pos(lblWubbaStatus, DISP_W - 100, UI_MAIN_Y + 4);
    lv_obj_set_style_text_color(lblWubbaStatus, colGray, 0);
    lv_label_set_text(lblWubbaStatus, "STOPPED");

    lv_obj_t* lblGpsLabel = lv_label_create(scrWubba);
    lv_obj_set_pos(lblGpsLabel, 16, UI_MAIN_Y + 10);
    lv_obj_set_style_text_color(lblGpsLabel, colGray, 0);
    lv_label_set_text(lblGpsLabel, "GPS Status:");

    lblWubbaGps = lv_label_create(scrWubba);
    lv_obj_set_pos(lblWubbaGps, 120, UI_MAIN_Y + 10);
    lv_obj_set_style_text_color(lblWubbaGps, colRed, 0);
    lv_label_set_text(lblWubbaGps, "NO FIX");

    lv_obj_t* lblCoordsLabel = lv_label_create(scrWubba);
    lv_obj_set_pos(lblCoordsLabel, 16, UI_MAIN_Y + 35);
    lv_obj_set_style_text_color(lblCoordsLabel, colGray, 0);
    lv_label_set_text(lblCoordsLabel, "Position:");

    lblWubbaCoords = lv_label_create(scrWubba);
    lv_obj_set_pos(lblWubbaCoords, 16, UI_MAIN_Y + 55);
    lv_obj_set_style_text_color(lblWubbaCoords, colCyan, 0);
    lv_obj_set_style_text_font(lblWubbaCoords, &lv_font_montserrat_20, 0);
    lv_label_set_text(lblWubbaCoords, "---.---- N\n---.---- E");

    lv_obj_t* lblLoggedLabel = lv_label_create(scrWubba);
    lv_obj_set_pos(lblLoggedLabel, 16, UI_MAIN_Y + 105);
    lv_obj_set_style_text_color(lblLoggedLabel, colGray, 0);
    lv_label_set_text(lblLoggedLabel, "Networks logged:");

    lblWubbaLogged = lv_label_create(scrWubba);
    lv_obj_set_pos(lblWubbaLogged, 160, UI_MAIN_Y + 105);
    lv_obj_set_style_text_color(lblWubbaLogged, colGreen, 0);
    lv_label_set_text(lblWubbaLogged, "0");

    lv_obj_t* lblHint = lv_label_create(scrWubba);
    lv_obj_set_pos(lblHint, 16, UI_BOT_Y + 4);
    lv_obj_set_style_text_color(lblHint, colGray, 0);
    lv_label_set_text(lblHint, "Space: Start/Stop | B: Back | M: Menu");
}

// =============================================================================
// COUNCIL SCREEN (LoRa Mesh)
// =============================================================================
void createCouncilScreen() {
    scrCouncil = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scrCouncil, colBlack, 0);
    createStatusBar(scrCouncil, "[C] Council of Ricks");

    lblCouncilStatus = lv_label_create(scrCouncil);
    lv_obj_set_pos(lblCouncilStatus, DISP_W - 100, UI_MAIN_Y + 4);
    lv_obj_set_style_text_color(lblCouncilStatus, colGray, 0);
    lv_label_set_text(lblCouncilStatus, "OFFLINE");

    lv_obj_t* lblFreq = lv_label_create(scrCouncil);
    lv_obj_set_pos(lblFreq, 16, UI_MAIN_Y + 10);
    lv_obj_set_style_text_color(lblFreq, colCyan, 0);
    lv_label_set_text_fmt(lblFreq, "Freq: %.1f MHz | SF9 | 125kHz", LORA_FREQ);

    lblCouncilStats = lv_label_create(scrCouncil);
    lv_obj_set_pos(lblCouncilStats, 16, UI_MAIN_Y + 35);
    lv_obj_set_style_text_color(lblCouncilStats, colWhite, 0);
    lv_label_set_text(lblCouncilStats, "TX: 0 | RX: 0");

    lblCouncilRssi = lv_label_create(scrCouncil);
    lv_obj_set_pos(lblCouncilRssi, 16, UI_MAIN_Y + 55);
    lv_obj_set_style_text_color(lblCouncilRssi, colGray, 0);
    lv_label_set_text(lblCouncilRssi, "Last RSSI: ---");

    lv_obj_t* lblMsgLabel = lv_label_create(scrCouncil);
    lv_obj_set_pos(lblMsgLabel, 16, UI_MAIN_Y + 80);
    lv_obj_set_style_text_color(lblMsgLabel, colGray, 0);
    lv_label_set_text(lblMsgLabel, "Last message:");

    lblCouncilMsg = lv_label_create(scrCouncil);
    lv_obj_set_pos(lblCouncilMsg, 16, UI_MAIN_Y + 100);
    lv_obj_set_style_text_color(lblCouncilMsg, colYellow, 0);
    lv_label_set_text(lblCouncilMsg, "(none)");

    lv_obj_t* lblHint = lv_label_create(scrCouncil);
    lv_obj_set_pos(lblHint, 16, UI_BOT_Y + 4);
    lv_obj_set_style_text_color(lblHint, colGray, 0);
    lv_label_set_text(lblHint, "Space: Send Beacon | B: Back | M: Menu");
}

// =============================================================================
// PLUMBUS SCREEN (File Manager)
// =============================================================================
void createPlumbusScreen() {
    scrPlumbus = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scrPlumbus, colBlack, 0);
    createStatusBar(scrPlumbus, "[F] Plumbus Files");

    lblPlumbusStatus = lv_label_create(scrPlumbus);
    lv_obj_set_pos(lblPlumbusStatus, DISP_W - 100, UI_MAIN_Y + 4);
    lv_obj_set_style_text_color(lblPlumbusStatus, colGray, 0);
    lv_label_set_text(lblPlumbusStatus, "---");

    lblPlumbusSD = lv_label_create(scrPlumbus);
    lv_obj_set_pos(lblPlumbusSD, 16, UI_MAIN_Y + 10);
    lv_obj_set_style_text_color(lblPlumbusSD, colCyan, 0);
    lv_label_set_text(lblPlumbusSD, "SD Card: Checking...");

    for (int i = 0; i < 6; i++) {
        lblPlumbusFiles[i] = lv_label_create(scrPlumbus);
        lv_obj_set_pos(lblPlumbusFiles[i], 24, UI_MAIN_Y + 35 + i * 18);
        lv_obj_set_style_text_color(lblPlumbusFiles[i], colWhite, 0);
        lv_label_set_text(lblPlumbusFiles[i], "");
    }

    lv_obj_t* lblHint = lv_label_create(scrPlumbus);
    lv_obj_set_pos(lblHint, 16, UI_BOT_Y + 4);
    lv_obj_set_style_text_color(lblHint, colGray, 0);
    lv_label_set_text(lblHint, "R/Space: Refresh | B: Back | M: Menu");
}

// =============================================================================
// SETTINGS SCREEN
// =============================================================================
void createSettingsScreen() {
    scrSettings = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scrSettings, colBlack, 0);
    createStatusBar(scrSettings, "[*] Settings");

    const char* settingsLabels[] = {
        "Brightness",
        "LoRa Frequency",
        "WiFi Channel Hop",
        "BLE TX Power",
        "Save & Exit"
    };

    for (int i = 0; i < 5; i++) {
        lblSettingsItems[i] = lv_label_create(scrSettings);
        lv_obj_set_pos(lblSettingsItems[i], 24, UI_MAIN_Y + 15 + i * 28);
        lv_obj_set_style_text_color(lblSettingsItems[i], colWhite, 0);
        lv_label_set_text(lblSettingsItems[i], settingsLabels[i]);
    }

    lv_obj_t* lblHint = lv_label_create(scrSettings);
    lv_obj_set_pos(lblHint, 16, UI_BOT_Y + 4);
    lv_obj_set_style_text_color(lblHint, colGray, 0);
    lv_label_set_text(lblHint, "Rotate: Select | Space: Edit | B: Back | L: Backlight");
}

// =============================================================================
// SCREEN NAVIGATION
// =============================================================================
void gotoScreen(screen_t screen) {
    lv_obj_t* target = NULL;
    switch (screen) {
        case SCREEN_BOOT: target = scrBoot; break;
        case SCREEN_MENU: target = scrMenu; break;
        case SCREEN_PORTAL: target = scrPortal; break;
        case SCREEN_SCHWIFTY: target = scrSchwifty; break;
        case SCREEN_WUBBA_LUBBA: target = scrWubba; break;
        case SCREEN_COUNCIL: target = scrCouncil; break;
        case SCREEN_PLUMBUS: target = scrPlumbus; break;
        case SCREEN_SETTINGS: target = scrSettings; break;
        default: target = scrMenu; break;
    }
    if (target) {
        lv_scr_load(target);
        currentScreen = screen;
    }
}

// =============================================================================
// MENU NAVIGATION
// =============================================================================
void updateMenuHighlight() {
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        if (i == menuIndex) {
            lv_obj_set_style_text_color(lblMenuItems[i], colGreen, 0);
            lv_label_set_text_fmt(lblMenuItems[i], "> %s %s", MENU_ITEMS[i].icon, MENU_ITEMS[i].name);
        } else {
            lv_obj_set_style_text_color(lblMenuItems[i], colWhite, 0);
            lv_label_set_text_fmt(lblMenuItems[i], "  %s %s", MENU_ITEMS[i].icon, MENU_ITEMS[i].name);
        }
    }
    lv_label_set_text(lblMenuDesc, MENU_ITEMS[menuIndex].desc);
}

void menuNext() { menuIndex = (menuIndex + 1) % MENU_ITEM_COUNT; updateMenuHighlight(); }
void menuPrev() { menuIndex = (menuIndex - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT; updateMenuHighlight(); }
void menuSelect() { gotoScreen(MENU_ITEMS[menuIndex].screen); }

// =============================================================================
// WIFI SCANNER
// =============================================================================
void startWifiScan() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    wifiScanning = true;
    networkCount = 0;
    lv_label_set_text(lblPortalStatus, "SCANNING");
    lv_obj_set_style_text_color(lblPortalStatus, colGreen, 0);
}

void stopWifiScan() {
    wifiScanning = false;
    WiFi.mode(WIFI_OFF);
    lv_label_set_text(lblPortalStatus, "STOPPED");
    lv_obj_set_style_text_color(lblPortalStatus, colYellow, 0);
}

void updateWifiScan() {
    if (!wifiScanning) return;
    if (millis() - lastWifiScan < 500) return;
    lastWifiScan = millis();

    // Channel hopping
    if (millis() - lastChannelHop > WIFI_CHANNEL_HOP_MS) {
        lastChannelHop = millis();
        scanChannel = (scanChannel % 13) + 1;
        esp_wifi_set_channel(scanChannel, WIFI_SECOND_CHAN_NONE);
    }

    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_FAILED) {
        WiFi.scanNetworks(true, true);
    } else if (n >= 0) {
        networkCount = n;
        lv_label_set_text_fmt(lblPortalCount, "Networks: %d | Ch: %d", networkCount, scanChannel);

        for (int i = 0; i < 8; i++) {
            if (i < n) {
                String ssid = WiFi.SSID(i);
                if (ssid.length() == 0) ssid = "<hidden>";
                if (ssid.length() > 18) ssid = ssid.substring(0, 15) + "...";
                int8_t rssi = WiFi.RSSI(i);
                const char* auth = WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "O" : "E";
                lv_label_set_text_fmt(lblPortalNetworks[i], "%s %ddB [%s]", ssid.c_str(), rssi, auth);
                lv_obj_set_style_text_color(lblPortalNetworks[i],
                    rssi > -50 ? colGreen : rssi > -70 ? colYellow : colRed, 0);
            } else {
                lv_label_set_text(lblPortalNetworks[i], "");
            }
        }
        WiFi.scanDelete();
        WiFi.scanNetworks(true, true);
        totalXP += n;
    }
}

// =============================================================================
// BLE SPAM
// =============================================================================
static NimBLEAdvertising* pAdvertising = nullptr;

void initBLE() {
    NimBLEDevice::init("RICK");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    pAdvertising = NimBLEDevice::getAdvertising();
}

void startBleSpam() {
    bleSpamming = true;
    bleSpamCount = 0;
    lv_label_set_text(lblSchwiftyStatus, "ACTIVE");
    lv_obj_set_style_text_color(lblSchwiftyStatus, colGreen, 0);
}

void stopBleSpam() {
    bleSpamming = false;
    if (pAdvertising) pAdvertising->stop();
    lv_label_set_text(lblSchwiftyStatus, "STOPPED");
    lv_obj_set_style_text_color(lblSchwiftyStatus, colGray, 0);
}

void updateBleSpam() {
    if (!bleSpamming || !pAdvertising) return;
    if (millis() - lastBleSpam < BLE_SPAM_INTERVAL_MS) return;
    lastBleSpam = millis();

    uint8_t data[8];
    for (int i = 0; i < 8; i++) data[i] = random(256);

    NimBLEAdvertisementData advData;
    advData.setFlags(0x06);

    if (bleTarget == BLE_TARGET_APPLE || (bleTarget == BLE_TARGET_ALL && random(2))) {
        data[0] = 0x07; data[1] = 0x19; data[2] = 0x07; data[3] = 0x0e; data[4] = 0x20;
        advData.setManufacturerData(std::string((char*)data, 8));
    } else {
        advData.setServiceData(NimBLEUUID((uint16_t)0xFE2C), std::string((char*)data, 6));
    }

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->start();
    delay(10);
    pAdvertising->stop();

    bleSpamCount++;
    lv_label_set_text_fmt(lblSchwiftyCount, "%lu", bleSpamCount);
    if (bleSpamCount % 100 == 0) totalXP += XP_BLE_SPAM_100;
}

// =============================================================================
// GPS WARDRIVING
// =============================================================================
void startWardriving() {
    gpsActive = true;
    startWifiScan();
    lv_label_set_text(lblWubbaStatus, "ACTIVE");
    lv_obj_set_style_text_color(lblWubbaStatus, colGreen, 0);
}

void stopWardriving() {
    gpsActive = false;
    stopWifiScan();
    lv_label_set_text(lblWubbaStatus, "STOPPED");
    lv_obj_set_style_text_color(lblWubbaStatus, colGray, 0);
}

void updateGPS() {
    if (millis() - lastGpsUpdate < 1000) return;
    lastGpsUpdate = millis();

    // Process GPS data from LilyGoLib
    while (Serial1.available()) {
        instance.gps.encode(Serial1.read());
    }

    if (instance.gps.location.isValid()) {
        gpsFix = true;
        gpsLat = instance.gps.location.lat();
        gpsLon = instance.gps.location.lng();

        lv_label_set_text(lblWubbaGps, "FIX OK");
        lv_obj_set_style_text_color(lblWubbaGps, colGreen, 0);
        lv_label_set_text_fmt(lblWubbaCoords, "%.5f %c\n%.5f %c",
            fabs(gpsLat), gpsLat >= 0 ? 'N' : 'S',
            fabs(gpsLon), gpsLon >= 0 ? 'E' : 'W');

        if (gpsActive && networkCount > 0) {
            gpsNetworksLogged += networkCount;
            lv_label_set_text_fmt(lblWubbaLogged, "%lu", gpsNetworksLogged);
            totalXP += XP_GPS_WARDRIVING;
        }
    } else {
        gpsFix = false;
        lv_label_set_text(lblWubbaGps, "NO FIX");
        lv_obj_set_style_text_color(lblWubbaGps, colRed, 0);
    }
}

// =============================================================================
// LORA MESH
// =============================================================================
void initLoRa() {
    if (loraInitialized) return;
    // Use instance.initLoRa() which initializes the global 'radio' object
    if (instance.initLoRa()) {
        int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, 7, LORA_SYNC, LORA_TX_POWER, 8, 0, false);
        if (state == RADIOLIB_ERR_NONE) {
            loraInitialized = true;
            radio.startReceive();
        }
    }
}

void sendLoRaBeacon() {
    if (!loraInitialized) return;
    char beacon[32];
    snprintf(beacon, 32, "RICK-%04X BEACON", (uint16_t)random(0xFFFF));
    int state = radio.transmit((uint8_t*)beacon, strlen(beacon));
    if (state == RADIOLIB_ERR_NONE) {
        loraMsgSent++;
        lv_label_set_text_fmt(lblCouncilStats, "TX: %lu | RX: %lu", loraMsgSent, loraMsgRecv);
        lv_label_set_text(lblCouncilStatus, "TX OK");
        lv_obj_set_style_text_color(lblCouncilStatus, colGreen, 0);
        totalXP += XP_LORA_MESSAGE;
    }
    radio.startReceive();
}

void updateLoRa() {
    if (!loraInitialized) return;

    // Check for received packet using IRQ flags
    uint16_t irq = radio.getIrqFlags();
    if (irq & RADIOLIB_SX126X_IRQ_RX_DONE) {
        uint8_t buf[64];
        size_t len = 0;
        int state = radio.readData(buf, 64);
        if (state == RADIOLIB_ERR_NONE) {
            len = radio.getPacketLength();
            if (len > 0 && len < 64) {
                buf[len] = 0;
                loraMsgRecv++;
                loraLastRssi = radio.getRSSI();
                strncpy(loraLastMsg, (char*)buf, 63);
                lv_label_set_text_fmt(lblCouncilStats, "TX: %lu | RX: %lu", loraMsgSent, loraMsgRecv);
                lv_label_set_text_fmt(lblCouncilRssi, "Last RSSI: %d dBm", loraLastRssi);
                lv_label_set_text(lblCouncilMsg, loraLastMsg);
                lv_label_set_text(lblCouncilStatus, "RX");
                lv_obj_set_style_text_color(lblCouncilStatus, colCyan, 0);
            }
        }
        radio.clearIrqFlags(RADIOLIB_SX126X_IRQ_RX_DONE);
        radio.startReceive();
    }
}

// =============================================================================
// FILE MANAGER
// =============================================================================
void initSD() {
    sdCardReady = instance.installSD();
    if (sdCardReady) {
        sdTotalMB = SD.totalBytes() / (1024 * 1024);
        sdUsedMB = SD.usedBytes() / (1024 * 1024);
        lv_label_set_text_fmt(lblPlumbusSD, "SD: %luMB / %luMB", sdUsedMB, sdTotalMB);
        lv_label_set_text(lblPlumbusStatus, "READY");
        lv_obj_set_style_text_color(lblPlumbusStatus, colGreen, 0);
    } else {
        lv_label_set_text(lblPlumbusSD, "SD Card: Not found");
        lv_label_set_text(lblPlumbusStatus, "NO SD");
        lv_obj_set_style_text_color(lblPlumbusStatus, colRed, 0);
    }
}

void refreshFiles() {
    if (!sdCardReady) { initSD(); return; }

    File root = SD.open("/");
    int idx = 0;
    while (idx < 6) {
        File entry = root.openNextFile();
        if (!entry) break;
        lv_label_set_text_fmt(lblPlumbusFiles[idx], "%s %s (%luB)",
            entry.isDirectory() ? "[D]" : "[F]",
            entry.name(),
            entry.size());
        entry.close();
        idx++;
    }
    for (; idx < 6; idx++) lv_label_set_text(lblPlumbusFiles[idx], "");
    root.close();
    fileCount = idx;
}

// =============================================================================
// SETTINGS UPDATE
// =============================================================================
void updateSettingsDisplay() {
    const char* freqs[] = {"915 MHz (US)", "868 MHz (EU)"};
    lv_label_set_text_fmt(lblSettingsItems[0], "%sBrightness: %d/16",
        settingsIndex == 0 ? "> " : "  ", settingsBrightness);
    lv_label_set_text_fmt(lblSettingsItems[1], "%sLoRa: %s",
        settingsIndex == 1 ? "> " : "  ", freqs[settingsLoraFreq]);
    lv_label_set_text_fmt(lblSettingsItems[2], "%sWiFi Hop: %dms",
        settingsIndex == 2 ? "> " : "  ", WIFI_CHANNEL_HOP_MS);
    lv_label_set_text_fmt(lblSettingsItems[3], "%sBLE Power: +9dBm",
        settingsIndex == 3 ? "> " : "  ");
    lv_label_set_text_fmt(lblSettingsItems[4], "%sSave & Exit",
        settingsIndex == 4 ? "> " : "  ");

    for (int i = 0; i < 5; i++) {
        lv_obj_set_style_text_color(lblSettingsItems[i],
            i == settingsIndex ? colGreen : colWhite, 0);
    }
}

// =============================================================================
// HAPTIC FEEDBACK HELPER
// =============================================================================
void haptic(uint8_t effect) {
    instance.setHapticEffects(effect);
    instance.vibrator();
}

// =============================================================================
// XP AND RANK UPDATE
// =============================================================================
void updateXPDisplay() {
    // Calculate rank from XP
    rick_rank_t newRank = RANK_MORTY;
    for (int i = RANK_COUNT - 1; i >= 0; i--) {
        if (totalXP >= RANK_XP[i]) {
            newRank = (rick_rank_t)i;
            break;
        }
    }

    // Update rank if changed
    if (newRank != currentRank) {
        currentRank = newRank;
        haptic(HAPTIC_STRONG);  // Celebrate rank up!
    }

    // Update display
    if (lblXpBar) lv_label_set_text_fmt(lblXpBar, "XP: %lu", totalXP);
    if (lblRank) {
        lv_label_set_text(lblRank, RANK_NAMES[currentRank]);
        // Color based on rank tier
        lv_color_t rankColor = currentRank >= RANK_RICK ? colGreen :
                               currentRank >= RANK_BIRDPERSON ? colCyan :
                               currentRank >= RANK_BETH ? colYellow : colGray;
        lv_obj_set_style_text_color(lblRank, rankColor, 0);
    }
}

// =============================================================================
// KEYBOARD INPUT HANDLER
// =============================================================================
void handleKeyboard() {
    char c = 0;
    int state = instance.kb.getKey(&c);

    if (state != 1) return;  // No key pressed

    // Haptic feedback for keypress
    haptic(HAPTIC_LIGHT);

    // Convert to uppercase for easier matching
    char key = (c >= 'a' && c <= 'z') ? c - 32 : c;

    // Global navigation shortcuts (work from any screen)
    switch (key) {
        case 'P':  // Portal Gun - WiFi Scanner
            stopBleSpam(); stopWardriving();
            gotoScreen(SCREEN_PORTAL);
            return;
        case 'S':  // Get Schwifty - BLE Spam
            stopWifiScan(); stopWardriving();
            gotoScreen(SCREEN_SCHWIFTY);
            return;
        case 'W':  // Wubba Lubba - GPS Wardriving
            stopBleSpam();
            gotoScreen(SCREEN_WUBBA_LUBBA);
            return;
        case 'C':  // Council - LoRa Mesh
            stopWifiScan(); stopBleSpam(); stopWardriving();
            gotoScreen(SCREEN_COUNCIL);
            return;
        case 'F':  // Plumbus - File Manager
            stopWifiScan(); stopBleSpam(); stopWardriving();
            gotoScreen(SCREEN_PLUMBUS);
            return;
        case 'M':  // Menu
            stopWifiScan(); stopBleSpam(); stopWardriving();
            gotoScreen(SCREEN_MENU);
            return;
        case 'X':  // Settings
            stopWifiScan(); stopBleSpam(); stopWardriving();
            gotoScreen(SCREEN_SETTINGS);
            return;
        case 'B':  // Back to menu
        case 27:   // ESC
        case '$':  // Alt+4 on some keyboards
            stopWifiScan(); stopBleSpam(); stopWardriving();
            gotoScreen(SCREEN_MENU);
            return;
        case 'L':  // Toggle keyboard backlight
            kbBacklightOn = !kbBacklightOn;
            instance.kb.setBrightness(kbBacklightOn ? 127 : 0);
            return;
    }

    // Context-specific keys
    switch (currentScreen) {
        case SCREEN_MENU:
            if (key == '\n' || key == '\r' || key == ' ') {
                menuSelect();
            } else if (key == 'W' || c == 'w') {  // W = up in menu context
                menuPrev();
            } else if (key == 'Z' || c == 'z') {  // Z = down (or use number keys)
                menuNext();
            } else if (c >= '1' && c <= '6') {
                menuIndex = c - '1';
                updateMenuHighlight();
                menuSelect();
            }
            break;

        case SCREEN_PORTAL:
            if (key == ' ' || key == '\n' || key == '\r') {
                if (wifiScanning) stopWifiScan(); else startWifiScan();
            }
            break;

        case SCREEN_SCHWIFTY:
            if (key == ' ' || key == '\n' || key == '\r') {
                if (bleSpamming) stopBleSpam(); else startBleSpam();
            } else if (c >= '1' && c <= '5') {
                bleTarget = (ble_target_t)(c - '1');
                const char* targets[] = {"Apple", "Android", "Samsung", "Windows", "ALL"};
                lv_label_set_text_fmt(lblSchwiftyTarget, "Target: %s", targets[bleTarget]);
            }
            break;

        case SCREEN_WUBBA_LUBBA:
            if (key == ' ' || key == '\n' || key == '\r') {
                if (gpsActive) stopWardriving(); else startWardriving();
            }
            break;

        case SCREEN_COUNCIL:
            if (key == ' ' || key == '\n' || key == '\r') {
                if (!loraInitialized) initLoRa();
                sendLoRaBeacon();
            }
            break;

        case SCREEN_PLUMBUS:
            if (key == ' ' || key == '\n' || key == '\r' || key == 'R') {
                refreshFiles();
            }
            break;

        case SCREEN_SETTINGS:
            if (key == ' ' || key == '\n' || key == '\r') {
                if (settingsIndex == 0) {
                    settingsBrightness = (settingsBrightness % 16) + 1;
                    instance.setBrightness(settingsBrightness);
                } else if (settingsIndex == 1) {
                    settingsLoraFreq = !settingsLoraFreq;
                } else if (settingsIndex == 4) {
                    haptic(HAPTIC_STRONG);
                    gotoScreen(SCREEN_MENU);
                }
                updateSettingsDisplay();
            }
            break;

        default:
            break;
    }
}

// =============================================================================
// ROTARY INPUT HANDLING
// =============================================================================
void handleRotary() {
    static bool lastBtn = false;
    static uint32_t btnPressTime = 0;

    RotaryMsg_t rotary = instance.getRotary();

    // Rotary rotation
    if (rotary.dir != ROTARY_DIR_NONE) {
        // Haptic feedback for rotation
        haptic(HAPTIC_LIGHT);

        int dir = (rotary.dir == ROTARY_DIR_UP) ? 1 : -1;

        switch (currentScreen) {
            case SCREEN_MENU:
                if (dir > 0) menuNext(); else menuPrev();
                break;
            case SCREEN_SCHWIFTY: {
                bleTarget = (ble_target_t)((bleTarget + dir + BLE_TARGET_COUNT) % BLE_TARGET_COUNT);
                const char* targets[] = {"Apple", "Android", "Samsung", "Windows", "ALL"};
                lv_label_set_text_fmt(lblSchwiftyTarget, "Target: %s", targets[bleTarget]);
                break;
            }
            case SCREEN_SETTINGS:
                settingsIndex = (settingsIndex + dir + 5) % 5;
                updateSettingsDisplay();
                break;
            default:
                break;
        }
        instance.clearRotaryMsg();
    }

    // Button handling
    bool btn = rotary.centerBtnPressed;
    if (btn && !lastBtn) btnPressTime = millis();

    if (!btn && lastBtn) {
        uint32_t dur = millis() - btnPressTime;

        // Haptic feedback for button press
        haptic(dur > 800 ? HAPTIC_STRONG : HAPTIC_MEDIUM);

        if (dur > 800) {
            // Long press - back to menu
            stopWifiScan(); stopBleSpam(); stopWardriving();
            loraActive = false;
            gotoScreen(SCREEN_MENU);
        } else {
            // Short press - action
            switch (currentScreen) {
                case SCREEN_MENU: menuSelect(); break;
                case SCREEN_PORTAL:
                    if (wifiScanning) stopWifiScan(); else startWifiScan();
                    break;
                case SCREEN_SCHWIFTY:
                    if (bleSpamming) stopBleSpam(); else startBleSpam();
                    break;
                case SCREEN_WUBBA_LUBBA:
                    if (gpsActive) stopWardriving(); else startWardriving();
                    break;
                case SCREEN_COUNCIL:
                    if (!loraInitialized) initLoRa();
                    sendLoRaBeacon();
                    break;
                case SCREEN_PLUMBUS:
                    refreshFiles();
                    break;
                case SCREEN_SETTINGS:
                    if (settingsIndex == 0) {
                        settingsBrightness = (settingsBrightness % 16) + 1;
                        instance.setBrightness(settingsBrightness);
                    } else if (settingsIndex == 1) {
                        settingsLoraFreq = !settingsLoraFreq;
                    } else if (settingsIndex == 4) {
                        haptic(HAPTIC_STRONG);
                        gotoScreen(SCREEN_MENU);
                    }
                    updateSettingsDisplay();
                    break;
                default: break;
            }
        }
    }
    lastBtn = btn;
}

// =============================================================================
// COMBINED INPUT HANDLER
// =============================================================================
void handleInput() {
    handleKeyboard();
    handleRotary();
}

// =============================================================================
// STATUS UPDATE
// =============================================================================
void updateStatus() {
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate < 1000) return;
    lastUpdate = millis();

    int batt = instance.gauge.getStateOfCharge();
    bool charging = instance.ppm.isCharging();
    lv_label_set_text_fmt(lblBattery, "%d%%%s", batt, charging ? "+" : "");
    lv_obj_set_style_text_color(lblBattery,
        batt < 20 ? colRed : charging ? colGreen : colCyan, 0);

    uint32_t secs = millis() / 1000;
    lv_label_set_text_fmt(lblTime, "%02lu:%02lu", secs / 3600, (secs / 60) % 60);

    // Update XP display
    updateXPDisplay();
}

// =============================================================================
// SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n=== RICK v" RICK_VERSION " ===");
    Serial.println("Wubba Lubba Dub Dub!\n");

    // LilyGoLib
    Serial.println("[1] LilyGoLib...");
    if (!instance.begin()) { Serial.println("FAILED!"); while(1); }
    Serial.println("OK");

    // LVGL
    Serial.println("[2] LVGL...");
    beginLvglHelper(instance);
    initColors();
    Serial.println("OK");

    // BLE
    Serial.println("[3] BLE...");
    initBLE();
    Serial.println("OK");

    // GPS Serial
    Serial.println("[4] GPS...");
    Serial1.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    Serial.println("OK");

    // Brightness
    instance.setBrightness(settingsBrightness);

    // Create screens
    Serial.println("[5] UI...");
    createBootScreen();
    createMenuScreen();
    createPortalScreen();
    createSchwiftyScreen();
    createWubbaScreen();
    createCouncilScreen();
    createPlumbusScreen();
    createSettingsScreen();
    Serial.println("OK");

    // Boot
    lv_scr_load(scrBoot);
    lv_timer_handler();
    delay(2000);

    // Menu
    gotoScreen(SCREEN_MENU);
    updateMenuHighlight();

    Serial.println("\n=== READY ===\n");
}

// =============================================================================
// LOOP
// =============================================================================
void loop() {
    lv_timer_handler();
    handleInput();
    updateStatus();

    switch (currentScreen) {
        case SCREEN_PORTAL: updateWifiScan(); break;
        case SCREEN_SCHWIFTY: updateBleSpam(); break;
        case SCREEN_WUBBA_LUBBA: updateGPS(); if (gpsActive) updateWifiScan(); break;
        case SCREEN_COUNCIL: updateLoRa(); break;
        default: break;
    }

    delay(5);
}
