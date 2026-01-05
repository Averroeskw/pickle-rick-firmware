/**
 * @file config.h
 * @brief Pickle Rick Firmware Configuration
 *
 * WiFi Security Tool for LILYGO T-LoraPager (K257)
 * Inspired by M5PORKCHOP - Rick & Morty Space Theme
 *
 * Copyright (c) 2025 AVERROES Tech Manufacturing
 * "I'm Pickle Rick!" - Rick Sanchez
 */

#ifndef PICKLE_RICK_CONFIG_H
#define PICKLE_RICK_CONFIG_H

// =============================================================================
// FIRMWARE INFO
// =============================================================================
#ifndef PICKLE_RICK_VERSION
#define PICKLE_RICK_VERSION "1.0.0"
#endif

#ifndef PICKLE_RICK_CODENAME
#define PICKLE_RICK_CODENAME "WUBBA_LUBBA"
#endif

#define FIRMWARE_NAME "PICKLE RICK"
#define MANUFACTURER "AVERROES Tech"

// =============================================================================
// HARDWARE PINS (FROM K257 PAGER-AI BASE)
// =============================================================================
// Display - ST7796 (sizes defined in pins_arduino.h)
#ifndef DISP_WIDTH
#define DISP_WIDTH          222
#endif
#ifndef DISP_HEIGHT
#define DISP_HEIGHT         480
#endif
#define DISP_ROTATION       0
#define DISP_BL_PIN         42
#define DISP_CS_PIN         38
#define DISP_DC_PIN         37
#define DISP_BRIGHTNESS     12  // 0-16 range

// I2C Bus
#define I2C_SDA_PIN         3
#define I2C_SCL_PIN         2
#define I2C_FREQ_HZ         400000

// SPI Bus (shared: LoRa, SD, NFC, Display)
#define SPI_MOSI_PIN        34
#define SPI_MISO_PIN        33
#define SPI_SCK_PIN         35

// SD Card
#define SD_CS_PIN           21

// LoRa SX1262
#define LORA_CS_PIN         36
#define LORA_RST_PIN        47
#define LORA_BUSY_PIN       48
#define LORA_IRQ_PIN        14

// GPS (UART)
#define GPS_TX_PIN          12
#define GPS_RX_PIN          4
#define GPS_PPS_PIN         13

// Audio (ES8311 Codec)
#define I2S_WS_PIN          18
#define I2S_SCK_PIN         11
#define I2S_MCLK_PIN        10
#define I2S_SDOUT_PIN       45
#define I2S_SDIN_PIN        17

// Keyboard (TCA8418)
#define KB_INT_PIN          6
#define KB_BACKLIGHT_PIN    46

// Rotary Encoder
#define ROTARY_A_PIN        40
#define ROTARY_B_PIN        41
#define ROTARY_BTN_PIN      7

// Interrupts
#define RTC_INT_PIN         1
#define NFC_INT_PIN         5
#define SENSOR_INT_PIN      8
#define NFC_CS_PIN          39

// I2C Addresses
#define ES8311_I2C_ADDR     0x18
#define XL9555_I2C_ADDR     0x20
#define TCA8418_I2C_ADDR    0x34
#define DRV2605_I2C_ADDR    0x5A
#define BHI260_I2C_ADDR     0x28

// XL9555 GPIO Expander pins
#define XL_DRV_EN           0   // Haptic driver
#define XL_AMP_EN           1   // Speaker amp
#define XL_KB_RST           2   // Keyboard reset
#define XL_LORA_EN          3   // LoRa enable
#define XL_GPS_EN           4   // GPS enable
#define XL_NFC_EN           5   // NFC enable
#define XL_GPS_RST          7   // GPS reset
#define XL_KB_EN            8   // Keyboard enable
#define XL_GPIO_EN          9
#define XL_SD_DET           10
#define XL_SD_PULLEN        11
#define XL_SD_EN            12

// =============================================================================
// COLOR SCHEME (Portal/Space Theme)
// =============================================================================
#define COLOR_PORTAL_GREEN  0x39E7  // Rick's portal green (RGB565)
#define COLOR_PORTAL_BLUE   0x04BF  // Portal blue edge
#define COLOR_SPACE_BLACK   0x0000  // Space background
#define COLOR_STAR_WHITE    0xFFFF  // Stars
#define COLOR_RICK_HAIR     0xB7DF  // Rick's hair (light blue-grey)
#define COLOR_MORTY_YELLOW  0xFE60  // Morty's shirt
#define COLOR_DANGER_RED    0xF800  // Federation alert
#define COLOR_COUNCIL_GOLD  0xFE20  // Council of Ricks gold
#define COLOR_MEESEEKS_BLUE 0x5D9F  // Mr. Meeseeks blue
#define COLOR_PICKLE_GREEN  0x2EC4  // Pickle Rick body

// LVGL colors (32-bit ARGB)
#define LV_COLOR_PORTAL_GREEN   0xFF39E770
#define LV_COLOR_SPACE_BG       0xFF0A0A1A
#define LV_COLOR_STAR           0xFFFFFFFF
#define LV_COLOR_TEXT           0xFF39E770
#define LV_COLOR_ACCENT         0xFF00FFCC

// =============================================================================
// WIFI SECURITY CONFIGURATION
// =============================================================================
#define WIFI_CHANNEL_MIN        1
#define WIFI_CHANNEL_MAX        13
#define WIFI_SCAN_PASSIVE       true
#define WIFI_DEAUTH_REASON      1       // Unspecified reason
#define WIFI_DEAUTH_FRAMES      5       // Frames per burst

// Handshake capture
#define HANDSHAKE_TIMEOUT_MS    60000
#define PMKID_CAPTURE_ENABLED   true
#define EAPOL_BUFFER_SIZE       512
#define MAX_CAPTURED_HANDSHAKES 100

// Channel hopping
#define CHANNEL_HOP_INTERVAL_MS 200
#define CHANNEL_DWELL_TIME_MS   100

// MAC randomization
#define MAC_RANDOMIZE_ENABLED   true
#define MAC_RANDOMIZE_INTERVAL  30000   // 30 seconds

// =============================================================================
// BLE CONFIGURATION
// =============================================================================
#define BLE_ENABLED             true
#define BLE_SPAM_INTERVAL_MS    50
#define BLE_TX_POWER            9       // Max power

typedef enum {
    BLE_TARGET_APPLE = 0,       // AirPods/AirDrop spam
    BLE_TARGET_ANDROID,         // Fast pair spam
    BLE_TARGET_SAMSUNG,         // SmartTag spam
    BLE_TARGET_WINDOWS,         // Swift Pair spam
    BLE_TARGET_ALL
} ble_target_t;

// =============================================================================
// GPS & WARDRIVING
// =============================================================================
#define GPS_ENABLED             true
#define GPS_BAUD_RATE           9600
#define GPS_UPDATE_INTERVAL_MS  1000
#define WARDRIVING_ENABLED      true
#define WIGLE_CSV_HEADER        "MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type"

// =============================================================================
// LORA CONFIGURATION (K257 EXCLUSIVE FEATURE!)
// =============================================================================
#define LORA_ENABLED            true
#define LORA_FREQUENCY          915E6   // US frequency
#define LORA_BANDWIDTH          125000
#define LORA_SPREADING_FACTOR   7
#define LORA_CODING_RATE        5
#define LORA_SYNC_WORD          0x52    // 'R' for Rick
#define LORA_TX_POWER           22      // Max power dBm

// Mesh networking
#define LORA_MESH_ENABLED       true
#define LORA_MESH_MAX_NODES     16
#define LORA_MESH_BEACON_MS     10000

// =============================================================================
// SD CARD
// =============================================================================
#define SD_MOUNT_POINT          "/sd"
#define SD_LOG_ENABLED          true
#define SD_MAX_LOG_SIZE_MB      100

// Directories
#define DIR_ROOT                "/sd/pickle_rick"
#define DIR_HANDSHAKES          "/sd/pickle_rick/handshakes"
#define DIR_PMKID               "/sd/pickle_rick/pmkid"
#define DIR_WARDRIVING          "/sd/pickle_rick/wardriving"
#define DIR_LOGS                "/sd/pickle_rick/logs"
#define DIR_CONFIG              "/sd/pickle_rick/config"
#define DIR_XP                  "/sd/pickle_rick/xp"
#define DIR_ACHIEVEMENTS        "/sd/pickle_rick/achievements"

// =============================================================================
// GAMIFICATION SYSTEM (Rick & Morty Theme)
// =============================================================================
#define GAMIFICATION_ENABLED    true
#define XP_PERSIST_ENABLED      true

// XP Rewards
#define XP_NETWORK_FOUND        10
#define XP_HIDDEN_NETWORK       25
#define XP_HANDSHAKE_CAPTURED   100
#define XP_PMKID_EXTRACTED      150
#define XP_WARDRIVING_POINT     5
#define XP_BLE_SPAM_SUCCESS     25
#define XP_LORA_MESSAGE_SENT    20
#define XP_LORA_NODE_FOUND      50
#define XP_CHALLENGE_COMPLETE   200
#define XP_ACHIEVEMENT_UNLOCK   500

// Ranks (40 levels)
typedef enum {
    RANK_MORTY = 0,             // Level 1-2: Naive beginner
    RANK_SUMMER,                // Level 3-4: Social media expert
    RANK_BETH,                  // Level 5-6: Horse surgeon
    RANK_JERRY,                 // Level 7-8: Unemployed but trying
    RANK_MR_MEESEEKS,           // Level 9-10: "Look at me!"
    RANK_SCARY_TERRY,           // Level 11-12: "Bitch!"
    RANK_BIRDPERSON,            // Level 13-14: Loyal friend
    RANK_SQUANCHY,              // Level 15-16: Party animal
    RANK_UNITY,                 // Level 17-18: Collective mind
    RANK_KROMBOPULOS,           // Level 19-20: "Oh boy!"
    RANK_EVIL_MORTY,            // Level 21-22: Mastermind
    RANK_PICKLE_RICK,           // Level 23-24: "I'm Pickle Rick!"
    RANK_TOXIC_RICK,            // Level 25-26: Pure toxicity
    RANK_TINY_RICK,             // Level 27-28: "Tiny Rick!"
    RANK_DOOFUS_RICK,           // Level 29-30: Nicest Rick
    RANK_COUNCIL_RICK,          // Level 31-32: Council member
    RANK_RICK_PRIME,            // Level 33-34: OG Rick
    RANK_C137_RICK,             // Level 35-36: Our Rick
    RANK_PORTAL_MASTER,         // Level 37-38: Dimension hopper
    RANK_DIMENSION_HOPPER,      // Level 39
    RANK_MULTIVERSE_GOD,        // Level 40: Ultimate
    RANK_MAX = 40
} rick_rank_t;

// =============================================================================
// OPERATIONAL MODES (Rebranded from PORKCHOP)
// =============================================================================
typedef enum {
    MODE_MENU = 0,              // Main menu (Rick's garage)
    MODE_PORTAL,                // WiFi scanning - "Portal Gun"
    MODE_INTERDIMENSIONAL,      // Handshake capture - "Interdimensional Cable"
    MODE_SCHWIFTY,              // BLE spam - "Get Schwifty"
    MODE_WUBBA_LUBBA,           // Wardriving - "Wubba Lubba Dub Dub"
    MODE_CHILL,                 // Passive mode - "Chill with Unity"
    MODE_SPECTRUM,              // Spectrum analyzer - "Microverse"
    MODE_LORA_MESH,             // LoRa mesh - "Council of Ricks"
    MODE_PLUMBUS,               // File manager - "Plumbus Commander"
    MODE_SETTINGS,              // Settings - "Garage Workshop"
    MODE_COUNT
} operation_mode_t;

// Mode names for display
static const char* MODE_NAMES[] = {
    "RICK'S GARAGE",
    "PORTAL GUN",
    "INTERDIMENSIONAL",
    "GET SCHWIFTY",
    "WUBBA LUBBA",
    "CHILL MODE",
    "MICROVERSE",
    "COUNCIL OF RICKS",
    "PLUMBUS CMD",
    "WORKSHOP"
};

// =============================================================================
// MOOD SYSTEM (Rick's mood = performance buffs)
// =============================================================================
typedef enum {
    MOOD_GENIUS = 0,            // Sober genius: +20% XP
    MOOD_DRUNK,                 // Classic Rick: Normal
    MOOD_ANGRY,                 // Angry Rick: +10% scan speed
    MOOD_DEPRESSED,             // "Wubba lubba dub dub": -10%
    MOOD_PICKLE,                // Pickle Rick: +50% XP!
    MOOD_WUBBA_LUBBA            // Random buffs
} rick_mood_t;

// =============================================================================
// APPLICATION STATES
// =============================================================================
typedef enum {
    STATE_BOOT = 0,
    STATE_MENU,
    STATE_SCANNING,
    STATE_CAPTURING,
    STATE_ATTACKING,
    STATE_WARDRIVING,
    STATE_LORA_ACTIVE,
    STATE_IDLE,
    STATE_ERROR,
    STATE_SLEEP
} app_state_t;

// =============================================================================
// CHALLENGE SYSTEM
// =============================================================================
#define DAILY_CHALLENGES        3
#define CHALLENGE_REFRESH_HOURS 24

typedef enum {
    CHALLENGE_SCAN_NETWORKS = 0,    // "Scan 50 networks"
    CHALLENGE_CAPTURE_HANDSHAKE,    // "Capture 3 handshakes"
    CHALLENGE_WARDRIVE_DISTANCE,    // "Wardrive 1km"
    CHALLENGE_BLE_SPAM_COUNT,       // "Spam 100 devices"
    CHALLENGE_FIND_HIDDEN_AP,       // "Find 5 hidden APs"
    CHALLENGE_EXTRACT_PMKID,        // "Extract 3 PMKIDs"
    CHALLENGE_LORA_MESSAGE,         // "Send 10 LoRa messages"
    CHALLENGE_SURVIVE_PORTAL,       // "Run Portal mode 10 mins"
    CHALLENGE_GET_SCHWIFTY,         // "BLE spam all target types"
    CHALLENGE_COUNT
} challenge_type_t;

// =============================================================================
// ACHIEVEMENTS (63 total, Rick & Morty themed)
// =============================================================================
#define ACHIEVEMENT_COUNT       63

// Achievement IDs
typedef enum {
    ACH_WUBBA_LUBBA = 0,        // First boot
    ACH_IM_PICKLE_RICK,         // Capture 100 handshakes
    ACH_GET_SCHWIFTY,           // BLE spam 1000 devices
    ACH_PORTAL_HOPPER,          // Scan in 5 locations
    ACH_COUNCIL_OF_RICKS,       // Max rank
    ACH_PLUMBUS_MASTER,         // Manage 100 files
    ACH_MEESEEKS_BOX,           // Complete 50 challenges
    ACH_MICROVERSE,             // 1 hour spectrum mode
    ACH_INTERDIMENSIONAL,       // Capture every auth type
    ACH_SHOW_ME_WHAT_YOU_GOT,   // First handshake
    ACH_TINY_RICK,              // Use tiny mode (power save)
    ACH_SCARY_TERRY,            // Deauth 100 clients
    ACH_BIRD_PERSON,            // 100 LoRa messages
    ACH_SQUANCHY,               // Party mode (all modes used)
    ACH_EVIL_MORTY,             // Find rogue AP
    // ... more achievements
    ACH_COUNT = 63
} achievement_id_t;

// =============================================================================
// INPUT CONFIGURATION
// =============================================================================
#define KEYBOARD_ENABLED        true
#define ROTARY_ENABLED          true
#define HAPTIC_ENABLED          true
#define HAPTIC_DURATION_MS      70

// Haptic effects (DRV2605)
#define HAPTIC_CLICK            1
#define HAPTIC_DOUBLE_CLICK     7
#define HAPTIC_TRIPLE_CLICK     14
#define HAPTIC_SOFT_BUMP        52
#define HAPTIC_BUZZ             58

// Key bindings
#define KEY_MENU_ESC            0x1B
#define KEY_SELECT_ENTER        0x0D
#define KEY_BACK_BACKSPACE      0x08
#define KEY_ACTION_SPACE        ' '

// =============================================================================
// POWER MANAGEMENT
// =============================================================================
#define POWER_SAVE_ENABLED      true
#define SCREEN_TIMEOUT_MS       60000
#define DEEP_SLEEP_TIMEOUT_MS   300000
#define BATTERY_LOW_PERCENT     15
#define BATTERY_CRITICAL_PCT    5

// =============================================================================
// AUDIO CONFIGURATION
// =============================================================================
#define AUDIO_ENABLED           true
#define AUDIO_SAMPLE_RATE       24000
#define AUDIO_BITS              16
#define AUDIO_CHANNELS          1
#define AUDIO_FRAME_MS          60
#define AUDIO_FRAME_BYTES       (AUDIO_SAMPLE_RATE * AUDIO_BITS / 8 * AUDIO_FRAME_MS / 1000)
#define SPK_BUFFER_MS           500

// Sound effects
#define SFX_PORTAL_OPEN         "/sd/sounds/portal.wav"
#define SFX_HANDSHAKE           "/sd/sounds/handshake.wav"
#define SFX_LEVEL_UP            "/sd/sounds/levelup.wav"
#define SFX_ACHIEVEMENT         "/sd/sounds/achievement.wav"
#define SFX_WUBBA_LUBBA         "/sd/sounds/wubba.wav"

#endif // PICKLE_RICK_CONFIG_H
