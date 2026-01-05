/**
 * @file ui_theme.cpp
 * @brief Pickle Rick Space Theme Implementation
 */

#include "ui_theme.h"

// =============================================================================
// STYLE DEFINITIONS
// =============================================================================
lv_style_t style_screen_bg;
lv_style_t style_panel;
lv_style_t style_panel_glow;
lv_style_t style_btn;
lv_style_t style_btn_pressed;
lv_style_t style_btn_focused;
lv_style_t style_label_title;
lv_style_t style_label_body;
lv_style_t style_label_small;
lv_style_t style_bar_bg;
lv_style_t style_bar_indicator;
lv_style_t style_list_item;
lv_style_t style_status_bar;

// =============================================================================
// THEME INITIALIZATION
// =============================================================================
void ui_theme_init(void) {
    // Screen background - deep space black
    lv_style_init(&style_screen_bg);
    lv_style_set_bg_color(&style_screen_bg, THEME_SPACE_BLACK);
    lv_style_set_bg_opa(&style_screen_bg, LV_OPA_COVER);

    // Panel - dark space with subtle border
    lv_style_init(&style_panel);
    lv_style_set_bg_color(&style_panel, THEME_SPACE_DARK);
    lv_style_set_bg_opa(&style_panel, LV_OPA_COVER);
    lv_style_set_border_color(&style_panel, THEME_PORTAL_GREEN);
    lv_style_set_border_width(&style_panel, 1);
    lv_style_set_border_opa(&style_panel, LV_OPA_50);
    lv_style_set_radius(&style_panel, 8);
    lv_style_set_pad_all(&style_panel, 10);

    // Panel with portal glow
    lv_style_init(&style_panel_glow);
    lv_style_set_bg_color(&style_panel_glow, THEME_SPACE_DARK);
    lv_style_set_bg_opa(&style_panel_glow, LV_OPA_COVER);
    lv_style_set_border_color(&style_panel_glow, THEME_PORTAL_GREEN);
    lv_style_set_border_width(&style_panel_glow, 2);
    lv_style_set_border_opa(&style_panel_glow, LV_OPA_COVER);
    lv_style_set_radius(&style_panel_glow, 8);
    lv_style_set_pad_all(&style_panel_glow, 10);
    lv_style_set_shadow_color(&style_panel_glow, THEME_PORTAL_GREEN);
    lv_style_set_shadow_width(&style_panel_glow, 15);
    lv_style_set_shadow_opa(&style_panel_glow, LV_OPA_30);
    lv_style_set_shadow_spread(&style_panel_glow, 2);

    // Button - portal green outline
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, THEME_SPACE_DARK);
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_border_color(&style_btn, THEME_PORTAL_GREEN);
    lv_style_set_border_width(&style_btn, 2);
    lv_style_set_border_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_btn, 6);
    lv_style_set_pad_all(&style_btn, 8);
    lv_style_set_text_color(&style_btn, THEME_PORTAL_GREEN);

    // Button pressed
    lv_style_init(&style_btn_pressed);
    lv_style_set_bg_color(&style_btn_pressed, THEME_PORTAL_GREEN);
    lv_style_set_bg_opa(&style_btn_pressed, LV_OPA_COVER);
    lv_style_set_text_color(&style_btn_pressed, THEME_SPACE_BLACK);

    // Button focused
    lv_style_init(&style_btn_focused);
    lv_style_set_border_color(&style_btn_focused, THEME_PORTAL_CYAN);
    lv_style_set_border_width(&style_btn_focused, 3);
    lv_style_set_shadow_color(&style_btn_focused, THEME_PORTAL_CYAN);
    lv_style_set_shadow_width(&style_btn_focused, 10);
    lv_style_set_shadow_opa(&style_btn_focused, LV_OPA_50);

    // Title label - large portal green
    lv_style_init(&style_label_title);
    lv_style_set_text_color(&style_label_title, THEME_PORTAL_GREEN);
    lv_style_set_text_font(&style_label_title, &lv_font_montserrat_24);

    // Body label
    lv_style_init(&style_label_body);
    lv_style_set_text_color(&style_label_body, THEME_STAR_WHITE);
    lv_style_set_text_font(&style_label_body, &lv_font_montserrat_14);

    // Small label
    lv_style_init(&style_label_small);
    lv_style_set_text_color(&style_label_small, THEME_TEXT_SECONDARY);
    lv_style_set_text_font(&style_label_small, &lv_font_montserrat_12);

    // Progress bar background
    lv_style_init(&style_bar_bg);
    lv_style_set_bg_color(&style_bar_bg, THEME_SPACE_DARK);
    lv_style_set_bg_opa(&style_bar_bg, LV_OPA_COVER);
    lv_style_set_border_color(&style_bar_bg, THEME_PORTAL_GREEN);
    lv_style_set_border_width(&style_bar_bg, 1);
    lv_style_set_radius(&style_bar_bg, 4);

    // Progress bar indicator
    lv_style_init(&style_bar_indicator);
    lv_style_set_bg_color(&style_bar_indicator, THEME_PORTAL_GREEN);
    lv_style_set_bg_opa(&style_bar_indicator, LV_OPA_COVER);
    lv_style_set_radius(&style_bar_indicator, 3);

    // List item
    lv_style_init(&style_list_item);
    lv_style_set_bg_color(&style_list_item, THEME_SPACE_DARK);
    lv_style_set_bg_opa(&style_list_item, LV_OPA_COVER);
    lv_style_set_border_color(&style_list_item, THEME_PORTAL_GREEN);
    lv_style_set_border_width(&style_list_item, 1);
    lv_style_set_border_opa(&style_list_item, LV_OPA_30);
    lv_style_set_border_side(&style_list_item, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_pad_all(&style_list_item, 6);
    lv_style_set_text_color(&style_list_item, THEME_STAR_WHITE);

    // Status bar
    lv_style_init(&style_status_bar);
    lv_style_set_bg_color(&style_status_bar, THEME_SPACE_DARK);
    lv_style_set_bg_opa(&style_status_bar, LV_OPA_90);
    lv_style_set_border_color(&style_status_bar, THEME_PORTAL_GREEN);
    lv_style_set_border_width(&style_status_bar, 1);
    lv_style_set_border_side(&style_status_bar, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_pad_all(&style_status_bar, 4);
}

// =============================================================================
// THEME HELPERS
// =============================================================================
void ui_theme_apply_screen(lv_obj_t* screen) {
    lv_obj_add_style(screen, &style_screen_bg, 0);
}

lv_obj_t* ui_create_panel(lv_obj_t* parent, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* panel = lv_obj_create(parent);
    lv_obj_set_size(panel, w, h);
    lv_obj_add_style(panel, &style_panel, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    return panel;
}

lv_obj_t* ui_create_button(lv_obj_t* parent, const char* text, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style_btn_focused, LV_STATE_FOCUSED);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, THEME_PORTAL_GREEN, 0);

    return btn;
}

lv_obj_t* ui_create_label(lv_obj_t* parent, const char* text, const lv_font_t* font) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, THEME_PORTAL_GREEN, 0);
    return label;
}

lv_obj_t* ui_create_progress_bar(lv_obj_t* parent, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_size(bar, w, h);
    lv_obj_add_style(bar, &style_bar_bg, LV_PART_MAIN);
    lv_obj_add_style(bar, &style_bar_indicator, LV_PART_INDICATOR);
    lv_bar_set_range(bar, 0, 100);
    return bar;
}

void ui_add_portal_glow(lv_obj_t* obj) {
    lv_obj_add_style(obj, &style_panel_glow, 0);
}

// Star animation callback
static void star_anim_cb(void* var, int32_t v) {
    lv_obj_t* star = (lv_obj_t*)var;
    lv_obj_set_style_opa(star, v, 0);
}

void ui_create_starfield(lv_obj_t* parent) {
    // Create random stars
    for (int i = 0; i < 20; i++) {
        lv_obj_t* star = lv_obj_create(parent);
        lv_obj_remove_style_all(star);
        lv_obj_set_size(star, 2 + (i % 3), 2 + (i % 3));
        lv_obj_set_style_bg_color(star, THEME_STAR_WHITE, 0);
        lv_obj_set_style_bg_opa(star, LV_OPA_70, 0);
        lv_obj_set_style_radius(star, LV_RADIUS_CIRCLE, 0);

        // Random position
        lv_coord_t x = (i * 37 + 13) % lv_obj_get_width(parent);
        lv_coord_t y = (i * 23 + 7) % lv_obj_get_height(parent);
        lv_obj_set_pos(star, x, y);

        // Twinkle animation
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, star);
        lv_anim_set_values(&a, LV_OPA_30, LV_OPA_COVER);
        lv_anim_set_time(&a, 1000 + (i * 200) % 2000);
        lv_anim_set_playback_time(&a, 1000 + (i * 200) % 2000);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_exec_cb(&a, star_anim_cb);
        lv_anim_set_delay(&a, i * 100);
        lv_anim_start(&a);
    }
}
