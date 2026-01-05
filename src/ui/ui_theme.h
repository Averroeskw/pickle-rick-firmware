/**
 * @file ui_theme.h
 * @brief Pickle Rick Space Theme for LVGL
 *
 * Portal green, space black, interdimensional vibes
 */

#ifndef UI_THEME_H
#define UI_THEME_H

#include <lvgl.h>

// =============================================================================
// SPACE THEME COLORS (RGB565 for LVGL)
// =============================================================================

// Primary Colors
#define THEME_PORTAL_GREEN      lv_color_hex(0x39E770)  // Rick's portal green
#define THEME_PORTAL_CYAN       lv_color_hex(0x00FFCC)  // Portal edge glow
#define THEME_SPACE_BLACK       lv_color_hex(0x0A0A1A)  // Deep space background
#define THEME_SPACE_DARK        lv_color_hex(0x151525)  // Panel background
#define THEME_STAR_WHITE        lv_color_hex(0xFFFFFF)  // Stars/text

// Accent Colors
#define THEME_RICK_HAIR         lv_color_hex(0xB0C4DE)  // Rick's hair blue-grey
#define THEME_MORTY_YELLOW      lv_color_hex(0xFFD700)  // Morty's shirt
#define THEME_DANGER_RED        lv_color_hex(0xFF4444)  // Danger/Federation
#define THEME_COUNCIL_GOLD      lv_color_hex(0xFFAA00)  // Council of Ricks
#define THEME_MEESEEKS_BLUE     lv_color_hex(0x5599FF)  // Mr. Meeseeks
#define THEME_PICKLE_GREEN      lv_color_hex(0x2EC44E)  // Pickle Rick body

// Status Colors
#define THEME_SUCCESS           lv_color_hex(0x00FF88)
#define THEME_WARNING           lv_color_hex(0xFFAA00)
#define THEME_ERROR             lv_color_hex(0xFF4444)
#define THEME_INFO              lv_color_hex(0x4488FF)

// UI Element Colors
#define THEME_BG_PRIMARY        THEME_SPACE_BLACK
#define THEME_BG_SECONDARY      THEME_SPACE_DARK
#define THEME_TEXT_PRIMARY      THEME_PORTAL_GREEN
#define THEME_TEXT_SECONDARY    lv_color_hex(0x88AA88)
#define THEME_ACCENT            THEME_PORTAL_CYAN
#define THEME_BORDER            lv_color_hex(0x39E770)

// =============================================================================
// STYLE DECLARATIONS
// =============================================================================

// Base styles
extern lv_style_t style_screen_bg;      // Screen background
extern lv_style_t style_panel;          // Panel/card background
extern lv_style_t style_panel_glow;     // Panel with portal glow
extern lv_style_t style_btn;            // Standard button
extern lv_style_t style_btn_pressed;    // Pressed button
extern lv_style_t style_btn_focused;    // Focused button
extern lv_style_t style_label_title;    // Title text
extern lv_style_t style_label_body;     // Body text
extern lv_style_t style_label_small;    // Small text
extern lv_style_t style_bar_bg;         // Progress bar background
extern lv_style_t style_bar_indicator;  // Progress bar fill
extern lv_style_t style_list_item;      // List item
extern lv_style_t style_status_bar;     // Top status bar

// =============================================================================
// THEME FUNCTIONS
// =============================================================================

/**
 * Initialize the space theme styles
 */
void ui_theme_init(void);

/**
 * Apply space theme to a screen
 */
void ui_theme_apply_screen(lv_obj_t* screen);

/**
 * Create a styled panel
 */
lv_obj_t* ui_create_panel(lv_obj_t* parent, lv_coord_t w, lv_coord_t h);

/**
 * Create a styled button with text
 */
lv_obj_t* ui_create_button(lv_obj_t* parent, const char* text, lv_coord_t w, lv_coord_t h);

/**
 * Create a styled label
 */
lv_obj_t* ui_create_label(lv_obj_t* parent, const char* text, const lv_font_t* font);

/**
 * Create a progress bar (XP bar style)
 */
lv_obj_t* ui_create_progress_bar(lv_obj_t* parent, lv_coord_t w, lv_coord_t h);

/**
 * Create portal glow effect (animated border)
 */
void ui_add_portal_glow(lv_obj_t* obj);

/**
 * Create star background animation
 */
void ui_create_starfield(lv_obj_t* parent);

#endif // UI_THEME_H
