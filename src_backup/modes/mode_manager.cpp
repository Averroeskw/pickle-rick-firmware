/**
 * @file mode_manager.cpp
 * @brief Mode Manager Implementation - Scroll Navigation
 */

#include "mode_manager.h"

// =============================================================================
// INITIALIZATION
// =============================================================================
void mode_init(mode_manager_t* mgr) {
    mgr->currentMode = MODE_MENU;
    mgr->selectedMode = MODE_MENU;
    mgr->inMenu = true;
    mgr->modeActive = false;
    mgr->menuIndex = 1;  // Start at first mode (Portal Gun)
    mgr->menuItemCount = MENU_MODE_COUNT;
    mgr->lastInputTime = millis();
    mgr->modeStartTime = 0;
}

// =============================================================================
// SCROLL NAVIGATION
// =============================================================================
void mode_scroll(mode_manager_t* mgr, int8_t direction) {
    mgr->lastInputTime = millis();

    if (mgr->inMenu) {
        // Navigate menu with scroll
        int newIndex = mgr->menuIndex + direction;

        // Wrap around
        if (newIndex < 1) {
            newIndex = mgr->menuItemCount - 1;
        } else if (newIndex >= mgr->menuItemCount) {
            newIndex = 1;
        }

        mgr->menuIndex = newIndex;
        mgr->selectedMode = (operation_mode_t)newIndex;

        Serial.printf("[MODE] Scroll to: %s (%d)\n",
                      MODE_INFO[mgr->menuIndex].name,
                      mgr->menuIndex);
    } else {
        // In-mode scrolling (mode-specific behavior)
        // This will be handled by individual mode implementations
    }
}

// =============================================================================
// SELECT
// =============================================================================
void mode_select(mode_manager_t* mgr) {
    mgr->lastInputTime = millis();

    if (mgr->inMenu) {
        // Enter selected mode
        mode_enter(mgr, mgr->selectedMode);
    } else {
        // In-mode select (mode-specific behavior)
    }
}

// =============================================================================
// BACK
// =============================================================================
void mode_back(mode_manager_t* mgr) {
    mgr->lastInputTime = millis();

    if (!mgr->inMenu) {
        // Exit current mode, return to menu
        mode_exit(mgr);
    }
}

// =============================================================================
// ENTER MODE
// =============================================================================
void mode_enter(mode_manager_t* mgr, operation_mode_t mode) {
    const mode_info_t* info = mode_get_info(mode);

    Serial.printf("[MODE] Entering: %s %s\n", info->icon, info->name);
    Serial.printf("[MODE] %s\n", info->description);

    mgr->currentMode = mode;
    mgr->inMenu = false;
    mgr->modeActive = true;
    mgr->modeStartTime = millis();

    // Mode-specific initialization will happen in main loop
}

// =============================================================================
// EXIT MODE
// =============================================================================
void mode_exit(mode_manager_t* mgr) {
    Serial.printf("[MODE] Exiting: %s\n", MODE_INFO[mgr->currentMode].name);

    mgr->currentMode = MODE_MENU;
    mgr->inMenu = true;
    mgr->modeActive = false;

    Serial.println("[MODE] Returned to Rick's Garage");
}

// =============================================================================
// INFO GETTERS
// =============================================================================
const mode_info_t* mode_get_info(operation_mode_t mode) {
    if (mode >= MODE_COUNT) return &MODE_INFO[0];
    return &MODE_INFO[mode];
}

const char* mode_get_name(mode_manager_t* mgr) {
    return MODE_INFO[mgr->currentMode].name;
}

const char* mode_get_icon(mode_manager_t* mgr) {
    return MODE_INFO[mgr->currentMode].icon;
}

bool mode_is_menu(mode_manager_t* mgr) {
    return mgr->inMenu;
}

uint8_t mode_get_menu_index(mode_manager_t* mgr) {
    return mgr->menuIndex;
}

// =============================================================================
// MODE TICK
// =============================================================================
void mode_tick(mode_manager_t* mgr) {
    // Update mode-specific state
    // Individual modes will implement their own tick functions
}
