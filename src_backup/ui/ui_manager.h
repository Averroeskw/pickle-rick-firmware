/**
 * @file ui_manager.h
 * @brief LVGL UI Manager - Space Theme with Rick Character
 *
 * Main UI framework using LVGL for K257's ST7796 display
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <lvgl.h>
#include "../config.h"
#include "../core/pickle_rick.h"

// =============================================================================
// UI CONSTANTS
// =============================================================================
#define UI_HEADER_HEIGHT        40
#define UI_FOOTER_HEIGHT        30
#define UI_SIDEBAR_WIDTH        60
#define UI_MENU_ITEM_HEIGHT     50
#define UI_AVATAR_SIZE_SMALL    32
#define UI_AVATAR_SIZE_MEDIUM   64
#define UI_AVATAR_SIZE_LARGE    128

// Animation durations
#define UI_ANIM_FAST            150
#define UI_ANIM_NORMAL          300
#define UI_ANIM_SLOW            500

// =============================================================================
// UI SCREENS
// =============================================================================
typedef enum {
    SCREEN_BOOT = 0,
    SCREEN_MENU,
    SCREEN_PORTAL,          // WiFi scanner
    SCREEN_INTERDIMENSIONAL,// Handshake capture
    SCREEN_SCHWIFTY,        // BLE spam
    SCREEN_WUBBA_LUBBA,     // Wardriving
    SCREEN_CHILL,           // Passive mode
    SCREEN_SPECTRUM,        // Spectrum analyzer
    SCREEN_LORA_MESH,       // LoRa mesh
    SCREEN_PLUMBUS,         // File manager
    SCREEN_SETTINGS,        // Settings
    SCREEN_ACHIEVEMENTS,    // Achievements
    SCREEN_STATS,           // Statistics
    SCREEN_COUNT
} ui_screen_t;

// =============================================================================
// UI STATE
// =============================================================================
typedef struct {
    ui_screen_t currentScreen;
    ui_screen_t previousScreen;
    lv_obj_t* screens[SCREEN_COUNT];
    lv_obj_t* header;
    lv_obj_t* footer;
    lv_obj_t* avatar;
    lv_obj_t* statusIcons;
    lv_obj_t* xpBar;
    lv_obj_t* notification;
    bool isTransitioning;
    rick_avatar_t* rick;
} ui_state_t;

// =============================================================================
// UI MANAGER FUNCTIONS
// =============================================================================

/**
 * Initialize LVGL and UI
 */
bool ui_init(ui_state_t* state, rick_avatar_t* rick);

/**
 * Create all screens
 */
void ui_create_screens(ui_state_t* state);

/**
 * Switch to screen with animation
 */
void ui_goto_screen(ui_state_t* state, ui_screen_t screen);

/**
 * Update UI - call in loop
 */
void ui_update(ui_state_t* state);

/**
 * Show notification popup
 */
void ui_show_notification(ui_state_t* state, const char* title, const char* message, uint16_t duration_ms);

/**
 * Show achievement unlock animation
 */
void ui_show_achievement(ui_state_t* state, const char* name, const char* icon);

/**
 * Show rank up animation
 */
void ui_show_rank_up(ui_state_t* state, rick_rank_t newRank);

/**
 * Update XP bar
 */
void ui_update_xp(ui_state_t* state, uint32_t xp, uint32_t xpForNext);

/**
 * Update status icons
 */
void ui_update_status(ui_state_t* state, bool wifi, bool ble, bool gps, bool lora, bool sd);

/**
 * Update battery indicator
 */
void ui_update_battery(ui_state_t* state, uint8_t percent, bool charging);

// =============================================================================
// SCREEN CREATORS
// =============================================================================

/**
 * Create boot splash screen
 */
lv_obj_t* ui_create_boot_screen(ui_state_t* state);

/**
 * Create main menu screen (Rick's Garage)
 */
lv_obj_t* ui_create_menu_screen(ui_state_t* state);

/**
 * Create Portal Gun screen (WiFi scanner)
 */
lv_obj_t* ui_create_portal_screen(ui_state_t* state);

/**
 * Create Interdimensional Cable screen (Handshake capture)
 */
lv_obj_t* ui_create_interdimensional_screen(ui_state_t* state);

/**
 * Create Get Schwifty screen (BLE spam)
 */
lv_obj_t* ui_create_schwifty_screen(ui_state_t* state);

/**
 * Create Wubba Lubba screen (Wardriving)
 */
lv_obj_t* ui_create_wubba_lubba_screen(ui_state_t* state);

/**
 * Create Microverse screen (Spectrum)
 */
lv_obj_t* ui_create_spectrum_screen(ui_state_t* state);

/**
 * Create Council of Ricks screen (LoRa mesh)
 */
lv_obj_t* ui_create_lora_screen(ui_state_t* state);

/**
 * Create Plumbus Commander screen (File manager)
 */
lv_obj_t* ui_create_plumbus_screen(ui_state_t* state);

/**
 * Create Settings screen
 */
lv_obj_t* ui_create_settings_screen(ui_state_t* state);

// =============================================================================
// UI COMPONENTS
// =============================================================================

/**
 * Create header bar with status icons
 */
lv_obj_t* ui_create_header(lv_obj_t* parent, ui_state_t* state);

/**
 * Create footer bar with XP and rank
 */
lv_obj_t* ui_create_footer(lv_obj_t* parent, ui_state_t* state);

/**
 * Create Rick avatar widget
 */
lv_obj_t* ui_create_avatar(lv_obj_t* parent, ui_state_t* state, uint8_t size);

/**
 * Create menu item button
 */
lv_obj_t* ui_create_menu_item(lv_obj_t* parent, const char* icon, const char* label, ui_screen_t target);

/**
 * Create network list item
 */
lv_obj_t* ui_create_network_item(lv_obj_t* parent, const char* ssid, int8_t rssi, uint8_t auth, bool captured);

/**
 * Create animated stars background
 */
void ui_create_starfield(lv_obj_t* parent);

/**
 * Create portal animation
 */
lv_obj_t* ui_create_portal(lv_obj_t* parent, int x, int y, int size);

// =============================================================================
// THEME & STYLES
// =============================================================================

/**
 * Apply Rick & Morty space theme
 */
void ui_apply_theme(ui_state_t* state);

/**
 * Get theme color
 */
lv_color_t ui_get_color(uint8_t colorId);

/**
 * Create custom font
 */
const lv_font_t* ui_get_font(uint8_t size);

#endif // UI_MANAGER_H
