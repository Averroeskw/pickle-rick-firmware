/**
 * @file mode_manager.h
 * @brief Mode Manager - Scroll Navigation Between Modes
 *
 * Handles mode transitions using rotary encoder scroll
 */

#ifndef MODE_MANAGER_H
#define MODE_MANAGER_H

#include <Arduino.h>
#include "../config.h"

// =============================================================================
// MODE MANAGER STATE
// =============================================================================
typedef struct {
    operation_mode_t currentMode;
    operation_mode_t selectedMode;
    bool inMenu;
    bool modeActive;
    uint8_t menuIndex;
    uint8_t menuItemCount;
    uint32_t lastInputTime;
    uint32_t modeStartTime;
} mode_manager_t;

// =============================================================================
// MODE INFO
// =============================================================================
typedef struct {
    operation_mode_t mode;
    const char* name;
    const char* icon;
    const char* description;
    bool requiresGPS;
    bool requiresSD;
    bool requiresLoRa;
} mode_info_t;

// Mode definitions - Using ASCII icons (emojis crash ESP32)
static const mode_info_t MODE_INFO[] = {
    {MODE_MENU,             "RICK'S GARAGE",        "[H]", "Main Menu",                    false, false, false},
    {MODE_PORTAL,           "PORTAL GUN",           "[P]", "WiFi Scanning",                false, false, false},
    {MODE_INTERDIMENSIONAL, "INTERDIMENSIONAL",     "[I]", "Handshake Capture",            false, true,  false},
    {MODE_SCHWIFTY,         "GET SCHWIFTY",         "[S]", "BLE Spam Attack",              false, false, false},
    {MODE_WUBBA_LUBBA,      "WUBBA LUBBA",          "[W]", "GPS Wardriving",               true,  true,  false},
    {MODE_CHILL,            "CHILL MODE",           "[C]", "Passive Observation",          false, false, false},
    {MODE_SPECTRUM,         "MICROVERSE",           "[M]", "Spectrum Analyzer",            false, false, false},
    {MODE_LORA_MESH,        "COUNCIL OF RICKS",     "[R]", "LoRa Mesh Network",            false, false, true},
    {MODE_PLUMBUS,          "PLUMBUS CMD",          "[F]", "File Manager",                 false, true,  false},
    {MODE_SETTINGS,         "WORKSHOP",             "[*]", "Settings",                     false, false, false},
};

#define MENU_MODE_COUNT 10

// =============================================================================
// MODE MANAGER FUNCTIONS
// =============================================================================

/**
 * Initialize mode manager
 */
void mode_init(mode_manager_t* mgr);

/**
 * Handle scroll input (rotary encoder)
 * @param direction: 1 = scroll down/next, -1 = scroll up/prev
 */
void mode_scroll(mode_manager_t* mgr, int8_t direction);

/**
 * Handle select input (rotary press or enter)
 */
void mode_select(mode_manager_t* mgr);

/**
 * Handle back input (ESC key)
 */
void mode_back(mode_manager_t* mgr);

/**
 * Enter specific mode
 */
void mode_enter(mode_manager_t* mgr, operation_mode_t mode);

/**
 * Exit current mode, return to menu
 */
void mode_exit(mode_manager_t* mgr);

/**
 * Get current mode info
 */
const mode_info_t* mode_get_info(operation_mode_t mode);

/**
 * Get current mode name
 */
const char* mode_get_name(mode_manager_t* mgr);

/**
 * Get current mode icon
 */
const char* mode_get_icon(mode_manager_t* mgr);

/**
 * Check if in menu
 */
bool mode_is_menu(mode_manager_t* mgr);

/**
 * Get menu index for rendering
 */
uint8_t mode_get_menu_index(mode_manager_t* mgr);

/**
 * Mode tick - update mode state
 */
void mode_tick(mode_manager_t* mgr);

#endif // MODE_MANAGER_H
