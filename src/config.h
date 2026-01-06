/**
 * @file config.h
 * @brief RICK Firmware Configuration
 *
 * WiFi Security Tool for LILYGO T-LoraPager (K257)
 * Inspired by M5PORKCHOP - Rick & Morty Space Theme
 * Created for an alter Dimension By Archie
 * "Wubba Lubba Dub Dub!" - Rick Sanchez
 */

#ifndef RICK_CONFIG_H
#define RICK_CONFIG_H

// =============================================================================
// FIRMWARE INFO
// =============================================================================
#ifndef RICK_VERSION
#define RICK_VERSION "1.0.0"
#endif

#ifndef RICK_CODENAME
#define RICK_CODENAME "WUBBA_LUBBA"
#endif

#define FIRMWARE_NAME "RICK"
#define MANUFACTURER "Archie"

// =============================================================================
// DISPLAY (from pins_arduino.h, managed by LilyGoLib)
// =============================================================================
#define DISP_W 480
#define DISP_H 222
#define DISP_BRIGHTNESS_DEFAULT 12  // 1-16 range

// UI Layout
#define UI_BAR_H 24
#define UI_MAIN_Y (UI_BAR_H + 1)
#define UI_MAIN_H (DISP_H - UI_BAR_H * 2 - 2)
#define UI_BOT_Y (DISP_H - UI_BAR_H)

// =============================================================================
// WIFI SCANNER SETTINGS
// =============================================================================
#define WIFI_MAX_NETWORKS       100
#define WIFI_CHANNEL_HOP_MS     200
#define WIFI_SCAN_TIMEOUT_MS    5000

// =============================================================================
// BLE SPAM SETTINGS
// =============================================================================
typedef enum {
    BLE_TARGET_APPLE = 0,
    BLE_TARGET_ANDROID,
    BLE_TARGET_SAMSUNG,
    BLE_TARGET_WINDOWS,
    BLE_TARGET_ALL,
    BLE_TARGET_COUNT
} ble_target_t;

#define BLE_SPAM_INTERVAL_MS    50
#define BLE_TX_POWER            9   // dBm

// =============================================================================
// LORA SETTINGS (Uses LilyGoLib instance.radio)
// =============================================================================
#define LORA_FREQ               915.0   // MHz (US) - change to 868.0 for EU
#define LORA_BW                 125.0   // kHz
#define LORA_SF                 9       // Spreading factor
#define LORA_SYNC               0x12    // Sync word
#define LORA_TX_POWER           20      // dBm

// =============================================================================
// GPS SETTINGS (Uses LilyGoLib instance.gps)
// =============================================================================
#define GPS_UPDATE_INTERVAL_MS  1000
#define GPS_FIX_TIMEOUT_MS      30000

// =============================================================================
// XP SYSTEM
// =============================================================================
typedef enum {
    RANK_MORTY = 0,         // Beginner
    RANK_JERRY,             // Amateur
    RANK_BETH,              // Competent
    RANK_SUMMER,            // Skilled
    RANK_BIRDPERSON,        // Expert
    RANK_SQUANCHY,          // Master
    RANK_EVIL_MORTY,        // Elite
    RANK_RICK,              // Legendary
    RANK_COUNCIL_RICK,      // Mythic
    RANK_COUNT
} rick_rank_t;

// XP thresholds for each rank
static const uint32_t RANK_XP[] = {
    0,          // Morty
    100,        // Jerry
    500,        // Beth
    1500,       // Summer
    5000,       // Birdperson
    15000,      // Squanchy
    50000,      // Evil Morty
    150000,     // Rick
    500000      // Council Rick
};

// Rank names
static const char* RANK_NAMES[] = {
    "Morty",
    "Jerry",
    "Beth",
    "Summer",
    "Birdperson",
    "Squanchy",
    "Evil Morty",
    "Rick",
    "Council Rick"
};

// XP awards
#define XP_NETWORK_FOUND        5
#define XP_HANDSHAKE_CAPTURED   50
#define XP_PMKID_CAPTURED       75
#define XP_HIDDEN_FOUND         25
#define XP_BLE_SPAM_100         10
#define XP_LORA_MESSAGE         15
#define XP_GPS_WARDRIVING       20

// =============================================================================
// THEME COLORS (Rick & Morty Portal Theme)
// =============================================================================
#define COLOR_PORTAL_GREEN      0x00FF00
#define COLOR_PORTAL_CYAN       0x00FFFF
#define COLOR_SPACE_BLACK       0x000000
#define COLOR_MORTY_YELLOW      0xFFD700
#define COLOR_RICK_BLUE         0x00CED1
#define COLOR_DANGER_RED        0xFF4444
#define COLOR_MUTED_GRAY        0x666666

// =============================================================================
// SCREENS
// =============================================================================
typedef enum {
    SCREEN_BOOT = 0,
    SCREEN_MENU,            // Rick's Garage - Main menu
    SCREEN_PORTAL,          // Portal Gun - WiFi Scanner
    SCREEN_INTERDIMENSIONAL,// Interdimensional Cable - Handshake capture
    SCREEN_SCHWIFTY,        // Get Schwifty - BLE Spam
    SCREEN_WUBBA_LUBBA,     // Wubba Lubba - Wardriving
    SCREEN_COUNCIL,         // Council of Ricks - LoRa Mesh
    SCREEN_PLUMBUS,         // Plumbus Commander - File Manager
    SCREEN_SETTINGS,        // Settings
    SCREEN_STATS,           // Statistics
    SCREEN_COUNT
} screen_t;

// =============================================================================
// MENU ITEMS
// =============================================================================
typedef struct {
    const char* name;
    const char* icon;
    const char* desc;
    screen_t screen;
} menu_item_t;

static const menu_item_t MENU_ITEMS[] = {
    {"Portal Gun",      "[P]", "WiFi Scanner",      SCREEN_PORTAL},
    {"Get Schwifty",    "[S]", "BLE Spam",          SCREEN_SCHWIFTY},
    {"Wubba Lubba",     "[W]", "GPS Wardriving",    SCREEN_WUBBA_LUBBA},
    {"Council",         "[C]", "LoRa Mesh",         SCREEN_COUNCIL},
    {"Plumbus",         "[F]", "File Manager",      SCREEN_PLUMBUS},
    {"Settings",        "[*]", "Configuration",     SCREEN_SETTINGS},
};
#define MENU_ITEM_COUNT (sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]))

#endif // RICK_CONFIG_H
