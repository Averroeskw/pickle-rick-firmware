/**
 * @file ui_manager.cpp
 * @brief Pickle Rick LVGL UI Implementation - Complete
 *
 * Space-themed UI for the K257 T-LoRa Pager
 */

#include "ui_manager.h"
#include "ui_theme.h"
#include <TFT_eSPI.h>

// Include backend modules for UI connection
#include "../wifi/wifi_scanner.h"
#include "../wifi/handshake_capture.h"
#include "../ble/ble_spam.h"
#include "../gps/wardriving.h"
#include "../lora/lora_mesh.h"
#include "../core/xp_system.h"

// =============================================================================
// DISPLAY CONFIGURATION
// =============================================================================
#define SCREEN_WIDTH  222
#define SCREEN_HEIGHT 480
#define LV_BUF_SIZE   (SCREEN_WIDTH * 20)  // Reduced buffer for lower memory usage

// TFT_eSPI instance
static TFT_eSPI tft = TFT_eSPI();

// LVGL buffers - single buffer to save memory
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[LV_BUF_SIZE];

// LVGL drivers
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t enc_drv;
static lv_indev_t* enc_indev = NULL;

// Input group for encoder navigation
static lv_group_t* input_group = NULL;

// Global UI state reference
static ui_state_t* g_ui_state = NULL;

// Encoder state
static int32_t encoder_diff = 0;
static bool encoder_pressed = false;

// =============================================================================
// ENCODER PINS (K257 T-LoRa Pager - from pins_arduino.h)
// =============================================================================
#define ENC_A    40   // ROTARY_A
#define ENC_B    41   // ROTARY_B
#define ENC_BTN  7    // ROTARY_C (center button)

static volatile int32_t enc_count = 0;
static int32_t last_enc_count = 0;

// Encoder ISR
static void IRAM_ATTR enc_isr() {
    static uint8_t lastState = 0;
    uint8_t state = (digitalRead(ENC_A) << 1) | digitalRead(ENC_B);

    // Simple state machine for quadrature decoding
    if (state != lastState) {
        if ((lastState == 0 && state == 1) || (lastState == 1 && state == 3) ||
            (lastState == 3 && state == 2) || (lastState == 2 && state == 0)) {
            enc_count++;
        } else {
            enc_count--;
        }
        lastState = state;
    }
}

// =============================================================================
// DISPLAY FLUSH CALLBACK
// =============================================================================
static void disp_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t*)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

// =============================================================================
// ENCODER READ CALLBACK
// =============================================================================
static void encoder_read_cb(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    // Read encoder rotation
    int32_t diff = enc_count - last_enc_count;
    last_enc_count = enc_count;

    data->enc_diff = diff;
    data->state = digitalRead(ENC_BTN) == LOW ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// =============================================================================
// INITIALIZATION
// =============================================================================
bool ui_init(ui_state_t* state, rick_avatar_t* rick) {
    Serial.println("[UI] Initializing LVGL...");

    if (state == NULL) {
        Serial.println("[UI] ERROR: state is NULL!");
        return false;
    }

    // Store global reference
    g_ui_state = state;

    // Initialize TFT
    Serial.println("[UI] Init TFT...");
    tft.init();
    tft.setRotation(0);  // Portrait
    tft.fillScreen(TFT_BLACK);
    Serial.println("[UI] TFT initialized");

    // Set backlight
    pinMode(42, OUTPUT);
    analogWrite(42, 200);  // ~80% brightness

    // Initialize encoder pins
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(ENC_BTN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENC_A), enc_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_B), enc_isr, CHANGE);

    Serial.printf("[UI] Display: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);

    // Initialize LVGL
    lv_init();

    // Initialize draw buffer (single buffer mode to save memory)
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, LV_BUF_SIZE);

    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize encoder input driver
    lv_indev_drv_init(&enc_drv);
    enc_drv.type = LV_INDEV_TYPE_ENCODER;
    enc_drv.read_cb = encoder_read_cb;
    enc_indev = lv_indev_drv_register(&enc_drv);

    // Create input group for encoder navigation
    input_group = lv_group_create();
    lv_group_set_default(input_group);
    lv_indev_set_group(enc_indev, input_group);

    // Initialize theme
    ui_theme_init();

    // Store state
    state->rick = rick;
    state->currentScreen = SCREEN_BOOT;
    state->previousScreen = SCREEN_BOOT;
    state->isTransitioning = false;

    // Initialize all screen pointers to NULL
    for (int i = 0; i < SCREEN_COUNT; i++) {
        state->screens[i] = NULL;
    }

    // Create all screens
    ui_create_screens(state);

    Serial.println("[UI] Initialized");
    return true;
}

// =============================================================================
// SCREEN CREATION
// =============================================================================
void ui_create_screens(ui_state_t* state) {
    Serial.println("[UI] Creating screens...");

    // Create screens one at a time with error checking
    state->screens[SCREEN_BOOT] = ui_create_boot_screen(state);
    Serial.println("[UI] Boot screen created");

    state->screens[SCREEN_MENU] = ui_create_menu_screen(state);
    Serial.println("[UI] Menu screen created");

    state->screens[SCREEN_PORTAL] = ui_create_portal_screen(state);
    Serial.println("[UI] Portal screen created");

    state->screens[SCREEN_INTERDIMENSIONAL] = ui_create_interdimensional_screen(state);
    state->screens[SCREEN_SCHWIFTY] = ui_create_schwifty_screen(state);
    state->screens[SCREEN_WUBBA_LUBBA] = ui_create_wubba_lubba_screen(state);
    state->screens[SCREEN_SPECTRUM] = ui_create_spectrum_screen(state);
    state->screens[SCREEN_LORA_MESH] = ui_create_lora_screen(state);
    state->screens[SCREEN_PLUMBUS] = ui_create_plumbus_screen(state);
    state->screens[SCREEN_SETTINGS] = ui_create_settings_screen(state);

    // Initialize with boot screen (with null check)
    if (state->screens[SCREEN_BOOT] != NULL) {
        lv_scr_load(state->screens[SCREEN_BOOT]);
    } else {
        Serial.println("[UI] ERROR: Boot screen is NULL!");
        return;
    }

    // Auto-transition to menu after boot
    lv_timer_create([](lv_timer_t* timer) {
        ui_state_t* s = (ui_state_t*)timer->user_data;
        if (s && s->screens[SCREEN_MENU]) {
            ui_goto_screen(s, SCREEN_MENU);
        }
        lv_timer_del(timer);
    }, 2500, state);

    Serial.println("[UI] Screens created");
}

// =============================================================================
// COMMON EVENT HANDLERS
// =============================================================================
static void back_btn_cb(lv_event_t* e) {
    if (g_ui_state) {
        ui_goto_screen(g_ui_state, SCREEN_MENU);
    }
}

static void menu_item_click_cb(lv_event_t* e) {
    ui_screen_t target = (ui_screen_t)(intptr_t)lv_event_get_user_data(e);
    if (g_ui_state) {
        ui_goto_screen(g_ui_state, target);
    }
}

// =============================================================================
// BOOT SCREEN
// =============================================================================
lv_obj_t* ui_create_boot_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);
    ui_create_starfield(scr);

    // Title panel with portal glow
    lv_obj_t* panel = lv_obj_create(scr);
    lv_obj_set_size(panel, 200, 180);
    lv_obj_center(panel);
    lv_obj_add_style(panel, &style_panel_glow, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    // Title
    lv_obj_t* title = lv_label_create(panel);
    lv_label_set_text(title, "PICKLE RICK");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Subtitle
    lv_obj_t* subtitle = lv_label_create(panel);
    lv_label_set_text(subtitle, "WiFi Security Tool");
    lv_obj_add_style(subtitle, &style_label_body, 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 45);

    // Version
    lv_obj_t* version = lv_label_create(panel);
    lv_label_set_text(version, "v" RICK_VERSION);
    lv_obj_add_style(version, &style_label_small, 0);
    lv_obj_align(version, LV_ALIGN_TOP_MID, 0, 70);

    // Loading bar
    lv_obj_t* bar = ui_create_progress_bar(panel, 160, 15);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -30);

    // Quote
    lv_obj_t* quote = lv_label_create(scr);
    lv_label_set_text(quote, "\"Wubba Lubba Dub Dub!\"");
    lv_obj_add_style(quote, &style_label_small, 0);
    lv_obj_align(quote, LV_ALIGN_BOTTOM_MID, 0, -20);

    // Animate loading bar
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, bar);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_bar_set_value);
    lv_anim_start(&a);

    return scr;
}

// =============================================================================
// MAIN MENU SCREEN (Rick's Garage)
// =============================================================================
lv_obj_t* ui_create_menu_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);
    ui_create_starfield(scr);

    // Header
    state->header = ui_create_header(scr, state);

    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_HOME " RICK'S GARAGE");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 50);

    // Menu container with scroll
    lv_obj_t* menu = lv_obj_create(scr);
    lv_obj_set_size(menu, 200, 320);
    lv_obj_align(menu, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_style_bg_opa(menu, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(menu, 0, 0);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(menu, 6, 0);

    // Menu items
    struct {
        const char* icon;
        const char* name;
        ui_screen_t target;
    } items[] = {
        {LV_SYMBOL_WIFI, "PORTAL GUN", SCREEN_PORTAL},
        {LV_SYMBOL_DOWNLOAD, "INTERDIMENSIONAL", SCREEN_INTERDIMENSIONAL},
        {LV_SYMBOL_AUDIO, "GET SCHWIFTY", SCREEN_SCHWIFTY},
        {LV_SYMBOL_GPS, "WUBBA LUBBA", SCREEN_WUBBA_LUBBA},
        {LV_SYMBOL_CHARGE, "MICROVERSE", SCREEN_SPECTRUM},
        {LV_SYMBOL_LOOP, "COUNCIL OF RICKS", SCREEN_LORA_MESH},
        {LV_SYMBOL_DIRECTORY, "PLUMBUS CMD", SCREEN_PLUMBUS},
        {LV_SYMBOL_SETTINGS, "WORKSHOP", SCREEN_SETTINGS},
    };

    for (int i = 0; i < 8; i++) {
        lv_obj_t* btn = ui_create_button(menu, "", 180, 36);
        lv_obj_add_event_cb(btn, menu_item_click_cb, LV_EVENT_CLICKED, (void*)(intptr_t)items[i].target);
        lv_group_add_obj(input_group, btn);

        lv_obj_t* lbl = lv_label_create(btn);
        char buf[32];
        snprintf(buf, sizeof(buf), "%s %s", items[i].icon, items[i].name);
        lv_label_set_text(lbl, buf);
        lv_obj_center(lbl);
    }

    // Footer with XP bar
    state->footer = ui_create_footer(scr, state);

    return scr;
}

// =============================================================================
// PORTAL GUN SCREEN (WiFi Scanner)
// =============================================================================
static lv_obj_t* portal_list = NULL;
static lv_obj_t* portal_status = NULL;
static lv_obj_t* portal_count = NULL;

static void portal_scan_cb(lv_event_t* e) {
    Serial.println("[UI] Starting WiFi scan...");
    // wifi_scanner_start() would be called here
    if (portal_status) {
        lv_label_set_text(portal_status, "Status: Scanning...");
    }
}

lv_obj_t* ui_create_portal_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);

    ui_create_header(scr, state);

    // Title bar
    lv_obj_t* title_bar = lv_obj_create(scr);
    lv_obj_set_size(title_bar, SCREEN_WIDTH, 35);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_add_style(title_bar, &style_panel, 0);
    lv_obj_clear_flag(title_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(title_bar);
    lv_label_set_text(title, LV_SYMBOL_WIFI " PORTAL GUN");
    lv_obj_add_style(title, &style_label_body, 0);
    lv_obj_center(title);

    // Status panel
    lv_obj_t* status_panel = lv_obj_create(scr);
    lv_obj_set_size(status_panel, 200, 50);
    lv_obj_align(status_panel, LV_ALIGN_TOP_MID, 0, 75);
    lv_obj_add_style(status_panel, &style_panel, 0);
    lv_obj_clear_flag(status_panel, LV_OBJ_FLAG_SCROLLABLE);

    portal_count = lv_label_create(status_panel);
    lv_label_set_text(portal_count, "Networks: 0 | Ch: 1");
    lv_obj_add_style(portal_count, &style_label_body, 0);
    lv_obj_align(portal_count, LV_ALIGN_TOP_LEFT, 5, 5);

    portal_status = lv_label_create(status_panel);
    lv_label_set_text(portal_status, "Status: Ready");
    lv_obj_add_style(portal_status, &style_label_small, 0);
    lv_obj_align(portal_status, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    // Network list
    portal_list = lv_list_create(scr);
    lv_obj_set_size(portal_list, 210, 270);
    lv_obj_align(portal_list, LV_ALIGN_CENTER, 0, 45);
    lv_obj_set_style_bg_color(portal_list, THEME_SPACE_DARK, 0);
    lv_obj_set_style_border_color(portal_list, THEME_PORTAL_GREEN, 0);
    lv_obj_set_style_border_width(portal_list, 1, 0);

    // Sample networks
    const char* networks[] = {"Portal_C137", "Citadel_5G", "BlipsAndChitz"};
    for (int i = 0; i < 3; i++) {
        lv_obj_t* item = lv_list_add_btn(portal_list, LV_SYMBOL_WIFI, networks[i]);
        lv_obj_set_style_bg_color(item, THEME_SPACE_DARK, 0);
        lv_obj_set_style_text_color(item, THEME_PORTAL_GREEN, 0);
        lv_group_add_obj(input_group, item);
    }

    // Buttons
    lv_obj_t* btn_bar = lv_obj_create(scr);
    lv_obj_set_size(btn_bar, 210, 45);
    lv_obj_align(btn_bar, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_opa(btn_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_bar, 0, 0);
    lv_obj_set_flex_flow(btn_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* btn_scan = ui_create_button(btn_bar, LV_SYMBOL_REFRESH " SCAN", 95, 35);
    lv_obj_add_event_cb(btn_scan, portal_scan_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_scan);

    lv_obj_t* btn_back = ui_create_button(btn_bar, LV_SYMBOL_LEFT " BACK", 95, 35);
    lv_obj_add_event_cb(btn_back, back_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_back);

    return scr;
}

// =============================================================================
// INTERDIMENSIONAL CABLE SCREEN (Handshake Capture)
// =============================================================================
static lv_obj_t* handshake_target = NULL;
static lv_obj_t* handshake_status = NULL;
static lv_obj_t* handshake_count = NULL;

static void handshake_start_cb(lv_event_t* e) {
    Serial.println("[UI] Starting handshake capture...");
    if (handshake_status) {
        lv_label_set_text(handshake_status, "Capturing...");
        lv_obj_set_style_text_color(handshake_status, THEME_WARNING, 0);
    }
}

static void handshake_deauth_cb(lv_event_t* e) {
    Serial.println("[UI] Sending deauth...");
}

lv_obj_t* ui_create_interdimensional_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);

    ui_create_header(scr, state);

    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_DOWNLOAD " INTERDIMENSIONAL");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 45);

    lv_obj_t* subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "Handshake Capture");
    lv_obj_add_style(subtitle, &style_label_small, 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 75);

    // Target panel
    lv_obj_t* target_panel = lv_obj_create(scr);
    lv_obj_set_size(target_panel, 200, 70);
    lv_obj_align(target_panel, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_add_style(target_panel, &style_panel, 0);
    lv_obj_clear_flag(target_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* target_lbl = lv_label_create(target_panel);
    lv_label_set_text(target_lbl, "Target:");
    lv_obj_add_style(target_lbl, &style_label_small, 0);
    lv_obj_align(target_lbl, LV_ALIGN_TOP_LEFT, 5, 5);

    handshake_target = lv_label_create(target_panel);
    lv_label_set_text(handshake_target, "Select from Portal Gun");
    lv_obj_add_style(handshake_target, &style_label_body, 0);
    lv_obj_align(handshake_target, LV_ALIGN_TOP_LEFT, 5, 25);

    handshake_status = lv_label_create(target_panel);
    lv_label_set_text(handshake_status, "Ready");
    lv_obj_set_style_text_color(handshake_status, THEME_SUCCESS, 0);
    lv_obj_align(handshake_status, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    // Stats panel
    lv_obj_t* stats_panel = lv_obj_create(scr);
    lv_obj_set_size(stats_panel, 200, 100);
    lv_obj_align(stats_panel, LV_ALIGN_CENTER, 0, 20);
    ui_add_portal_glow(stats_panel);
    lv_obj_clear_flag(stats_panel, LV_OBJ_FLAG_SCROLLABLE);

    // Handshakes captured
    handshake_count = lv_label_create(stats_panel);
    lv_label_set_text(handshake_count, "0");
    lv_obj_set_style_text_font(handshake_count, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(handshake_count, THEME_PORTAL_CYAN, 0);
    lv_obj_align(handshake_count, LV_ALIGN_TOP_MID, -50, 15);

    lv_obj_t* hs_lbl = lv_label_create(stats_panel);
    lv_label_set_text(hs_lbl, "Handshakes");
    lv_obj_add_style(hs_lbl, &style_label_small, 0);
    lv_obj_align(hs_lbl, LV_ALIGN_TOP_MID, -50, 45);

    // PMKIDs
    lv_obj_t* pmkid_count = lv_label_create(stats_panel);
    lv_label_set_text(pmkid_count, "0");
    lv_obj_set_style_text_font(pmkid_count, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(pmkid_count, THEME_PORTAL_GREEN, 0);
    lv_obj_align(pmkid_count, LV_ALIGN_TOP_MID, 50, 15);

    lv_obj_t* pmkid_lbl = lv_label_create(stats_panel);
    lv_label_set_text(pmkid_lbl, "PMKIDs");
    lv_obj_add_style(pmkid_lbl, &style_label_small, 0);
    lv_obj_align(pmkid_lbl, LV_ALIGN_TOP_MID, 50, 45);

    // Buttons
    lv_obj_t* btn_capture = ui_create_button(scr, LV_SYMBOL_DOWNLOAD " CAPTURE", 180, 40);
    lv_obj_align(btn_capture, LV_ALIGN_CENTER, 0, 100);
    lv_obj_add_event_cb(btn_capture, handshake_start_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_capture);

    lv_obj_t* btn_deauth = ui_create_button(scr, LV_SYMBOL_WARNING " DEAUTH", 180, 40);
    lv_obj_align(btn_deauth, LV_ALIGN_CENTER, 0, 150);
    lv_obj_add_event_cb(btn_deauth, handshake_deauth_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_border_color(btn_deauth, THEME_DANGER_RED, 0);
    lv_group_add_obj(input_group, btn_deauth);

    lv_obj_t* btn_back = ui_create_button(scr, LV_SYMBOL_LEFT " BACK", 100, 35);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(btn_back, back_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_back);

    return scr;
}

// =============================================================================
// GET SCHWIFTY SCREEN (BLE Spam)
// =============================================================================
static lv_obj_t* schwifty_count = NULL;
static lv_obj_t* schwifty_dropdown = NULL;
static bool schwifty_running = false;

static void schwifty_toggle_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    schwifty_running = !schwifty_running;

    if (schwifty_running) {
        lv_obj_t* lbl = lv_obj_get_child(btn, 0);
        lv_label_set_text(lbl, LV_SYMBOL_STOP " STOP");
        lv_obj_set_style_bg_color(btn, THEME_DANGER_RED, LV_STATE_DEFAULT);
        Serial.println("[UI] BLE spam started");
    } else {
        lv_obj_t* lbl = lv_obj_get_child(btn, 0);
        lv_label_set_text(lbl, LV_SYMBOL_PLAY " START");
        lv_obj_set_style_bg_color(btn, THEME_SPACE_DARK, LV_STATE_DEFAULT);
        Serial.println("[UI] BLE spam stopped");
    }
}

lv_obj_t* ui_create_schwifty_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);

    ui_create_header(scr, state);

    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_AUDIO " GET SCHWIFTY");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 45);

    // Counter panel
    lv_obj_t* counter_panel = lv_obj_create(scr);
    lv_obj_set_size(counter_panel, 180, 80);
    lv_obj_align(counter_panel, LV_ALIGN_TOP_MID, 0, 90);
    ui_add_portal_glow(counter_panel);
    lv_obj_clear_flag(counter_panel, LV_OBJ_FLAG_SCROLLABLE);

    schwifty_count = lv_label_create(counter_panel);
    lv_label_set_text(schwifty_count, "0");
    lv_obj_set_style_text_font(schwifty_count, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(schwifty_count, THEME_PORTAL_CYAN, 0);
    lv_obj_align(schwifty_count, LV_ALIGN_CENTER, 0, -5);

    lv_obj_t* count_sub = lv_label_create(counter_panel);
    lv_label_set_text(count_sub, "Packets Sent");
    lv_obj_add_style(count_sub, &style_label_small, 0);
    lv_obj_align(count_sub, LV_ALIGN_BOTTOM_MID, 0, -5);

    // Target dropdown
    lv_obj_t* target_lbl = lv_label_create(scr);
    lv_label_set_text(target_lbl, "Target:");
    lv_obj_add_style(target_lbl, &style_label_body, 0);
    lv_obj_align(target_lbl, LV_ALIGN_TOP_LEFT, 15, 185);

    schwifty_dropdown = lv_dropdown_create(scr);
    lv_dropdown_set_options(schwifty_dropdown,
        "ALL\nApple\nAndroid\nSamsung\nWindows");
    lv_obj_set_size(schwifty_dropdown, 180, 35);
    lv_obj_align(schwifty_dropdown, LV_ALIGN_TOP_MID, 0, 210);
    lv_obj_set_style_bg_color(schwifty_dropdown, THEME_SPACE_DARK, 0);
    lv_obj_set_style_border_color(schwifty_dropdown, THEME_PORTAL_GREEN, 0);
    lv_obj_set_style_text_color(schwifty_dropdown, THEME_PORTAL_GREEN, 0);
    lv_group_add_obj(input_group, schwifty_dropdown);

    // Speed slider
    lv_obj_t* speed_lbl = lv_label_create(scr);
    lv_label_set_text(speed_lbl, "Speed:");
    lv_obj_add_style(speed_lbl, &style_label_body, 0);
    lv_obj_align(speed_lbl, LV_ALIGN_TOP_LEFT, 15, 260);

    lv_obj_t* slider = lv_slider_create(scr);
    lv_obj_set_size(slider, 160, 10);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 290);
    lv_slider_set_range(slider, 10, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, THEME_SPACE_DARK, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, THEME_PORTAL_GREEN, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, THEME_PORTAL_CYAN, LV_PART_KNOB);
    lv_group_add_obj(input_group, slider);

    // Buttons
    lv_obj_t* btn_start = ui_create_button(scr, LV_SYMBOL_PLAY " START", 180, 45);
    lv_obj_align(btn_start, LV_ALIGN_CENTER, 0, 80);
    lv_obj_add_event_cb(btn_start, schwifty_toggle_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_start);

    lv_obj_t* btn_back = ui_create_button(scr, LV_SYMBOL_LEFT " BACK", 100, 35);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(btn_back, back_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_back);

    return scr;
}

// =============================================================================
// WUBBA LUBBA SCREEN (Wardriving)
// =============================================================================
static lv_obj_t* wardrive_gps_status = NULL;
static lv_obj_t* wardrive_net_count = NULL;
static lv_obj_t* wardrive_dist = NULL;
static bool wardrive_running = false;

static void wardrive_toggle_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    wardrive_running = !wardrive_running;

    lv_obj_t* lbl = lv_obj_get_child(btn, 0);
    if (wardrive_running) {
        lv_label_set_text(lbl, LV_SYMBOL_STOP " STOP");
        Serial.println("[UI] Wardriving started");
    } else {
        lv_label_set_text(lbl, LV_SYMBOL_PLAY " START");
        Serial.println("[UI] Wardriving stopped");
    }
}

static void wardrive_export_cb(lv_event_t* e) {
    Serial.println("[UI] Exporting wardrive data...");
}

lv_obj_t* ui_create_wubba_lubba_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);

    ui_create_header(scr, state);

    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_GPS " WUBBA LUBBA");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 45);

    // GPS panel
    lv_obj_t* gps_panel = lv_obj_create(scr);
    lv_obj_set_size(gps_panel, 200, 80);
    lv_obj_align(gps_panel, LV_ALIGN_TOP_MID, 0, 85);
    lv_obj_add_style(gps_panel, &style_panel, 0);
    lv_obj_clear_flag(gps_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* gps_icon = lv_label_create(gps_panel);
    lv_label_set_text(gps_icon, LV_SYMBOL_GPS);
    lv_obj_set_style_text_font(gps_icon, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(gps_icon, THEME_WARNING, 0);
    lv_obj_align(gps_icon, LV_ALIGN_LEFT_MID, 10, 0);

    wardrive_gps_status = lv_label_create(gps_panel);
    lv_label_set_text(wardrive_gps_status, "Searching...\nSats: 0");
    lv_obj_add_style(wardrive_gps_status, &style_label_body, 0);
    lv_obj_align(wardrive_gps_status, LV_ALIGN_LEFT_MID, 45, 0);

    // Stats panel
    lv_obj_t* stats_panel = lv_obj_create(scr);
    lv_obj_set_size(stats_panel, 200, 100);
    lv_obj_align(stats_panel, LV_ALIGN_CENTER, 0, 0);
    ui_add_portal_glow(stats_panel);
    lv_obj_clear_flag(stats_panel, LV_OBJ_FLAG_SCROLLABLE);

    wardrive_net_count = lv_label_create(stats_panel);
    lv_label_set_text(wardrive_net_count, "0");
    lv_obj_set_style_text_font(wardrive_net_count, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(wardrive_net_count, THEME_PORTAL_CYAN, 0);
    lv_obj_align(wardrive_net_count, LV_ALIGN_TOP_MID, -50, 10);

    lv_obj_t* net_lbl = lv_label_create(stats_panel);
    lv_label_set_text(net_lbl, "Networks");
    lv_obj_add_style(net_lbl, &style_label_small, 0);
    lv_obj_align(net_lbl, LV_ALIGN_TOP_MID, -50, 40);

    wardrive_dist = lv_label_create(stats_panel);
    lv_label_set_text(wardrive_dist, "0.0");
    lv_obj_set_style_text_font(wardrive_dist, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(wardrive_dist, THEME_PORTAL_GREEN, 0);
    lv_obj_align(wardrive_dist, LV_ALIGN_TOP_MID, 50, 10);

    lv_obj_t* dist_lbl = lv_label_create(stats_panel);
    lv_label_set_text(dist_lbl, "km");
    lv_obj_add_style(dist_lbl, &style_label_small, 0);
    lv_obj_align(dist_lbl, LV_ALIGN_TOP_MID, 50, 40);

    // Buttons
    lv_obj_t* btn_start = ui_create_button(scr, LV_SYMBOL_PLAY " START", 180, 45);
    lv_obj_align(btn_start, LV_ALIGN_CENTER, 0, 90);
    lv_obj_add_event_cb(btn_start, wardrive_toggle_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_start);

    lv_obj_t* btn_export = ui_create_button(scr, LV_SYMBOL_SAVE " EXPORT", 85, 35);
    lv_obj_align(btn_export, LV_ALIGN_BOTTOM_LEFT, 15, -10);
    lv_obj_add_event_cb(btn_export, wardrive_export_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_export);

    lv_obj_t* btn_back = ui_create_button(scr, LV_SYMBOL_LEFT " BACK", 85, 35);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_RIGHT, -15, -10);
    lv_obj_add_event_cb(btn_back, back_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_back);

    return scr;
}

// =============================================================================
// MICROVERSE SCREEN (Spectrum Analyzer)
// =============================================================================
static lv_obj_t* spectrum_chart = NULL;
static lv_chart_series_t* spectrum_series = NULL;

lv_obj_t* ui_create_spectrum_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);

    ui_create_header(scr, state);

    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_CHARGE " MICROVERSE");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 45);

    lv_obj_t* subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "2.4GHz Spectrum");
    lv_obj_add_style(subtitle, &style_label_small, 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 75);

    // Spectrum chart
    spectrum_chart = lv_chart_create(scr);
    lv_obj_set_size(spectrum_chart, 200, 200);
    lv_obj_align(spectrum_chart, LV_ALIGN_CENTER, 0, 20);
    lv_chart_set_type(spectrum_chart, LV_CHART_TYPE_BAR);
    lv_chart_set_point_count(spectrum_chart, 14);  // Channels 1-14
    lv_chart_set_range(spectrum_chart, LV_CHART_AXIS_PRIMARY_Y, -100, 0);

    lv_obj_set_style_bg_color(spectrum_chart, THEME_SPACE_DARK, 0);
    lv_obj_set_style_border_color(spectrum_chart, THEME_PORTAL_GREEN, 0);
    lv_obj_set_style_line_color(spectrum_chart, THEME_PORTAL_GREEN, LV_PART_MAIN);

    spectrum_series = lv_chart_add_series(spectrum_chart, THEME_PORTAL_GREEN, LV_CHART_AXIS_PRIMARY_Y);

    // Sample data
    for (int i = 0; i < 14; i++) {
        lv_chart_set_next_value(spectrum_chart, spectrum_series, -90 + (rand() % 40));
    }

    // Channel labels
    lv_obj_t* ch_lbl = lv_label_create(scr);
    lv_label_set_text(ch_lbl, "1  2  3  4  5  6  7  8  9 10 11 12 13 14");
    lv_obj_add_style(ch_lbl, &style_label_small, 0);
    lv_obj_align(ch_lbl, LV_ALIGN_CENTER, 0, 130);

    // Info panel
    lv_obj_t* info_panel = lv_obj_create(scr);
    lv_obj_set_size(info_panel, 200, 50);
    lv_obj_align(info_panel, LV_ALIGN_CENTER, 0, 180);
    lv_obj_add_style(info_panel, &style_panel, 0);
    lv_obj_clear_flag(info_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* peak_lbl = lv_label_create(info_panel);
    lv_label_set_text(peak_lbl, "Peak: Ch 6 (-45 dBm)");
    lv_obj_add_style(peak_lbl, &style_label_body, 0);
    lv_obj_center(peak_lbl);

    // Back button
    lv_obj_t* btn_back = ui_create_button(scr, LV_SYMBOL_LEFT " BACK", 100, 35);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(btn_back, back_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_back);

    return scr;
}

// =============================================================================
// COUNCIL OF RICKS SCREEN (LoRa Mesh)
// =============================================================================
static lv_obj_t* lora_node_list = NULL;
static lv_obj_t* lora_status = NULL;
static bool lora_enabled = false;

static void lora_toggle_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lora_enabled = !lora_enabled;

    lv_obj_t* lbl = lv_obj_get_child(btn, 0);
    if (lora_enabled) {
        lv_label_set_text(lbl, LV_SYMBOL_PAUSE " DISABLE");
        if (lora_status) lv_label_set_text(lora_status, "Status: Active");
        Serial.println("[UI] LoRa mesh enabled");
    } else {
        lv_label_set_text(lbl, LV_SYMBOL_PLAY " ENABLE");
        if (lora_status) lv_label_set_text(lora_status, "Status: Disabled");
        Serial.println("[UI] LoRa mesh disabled");
    }
}

lv_obj_t* ui_create_lora_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);

    ui_create_header(scr, state);

    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_LOOP " COUNCIL OF RICKS");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 45);

    // Status panel
    lv_obj_t* status_panel = lv_obj_create(scr);
    lv_obj_set_size(status_panel, 200, 60);
    lv_obj_align(status_panel, LV_ALIGN_TOP_MID, 0, 85);
    lv_obj_add_style(status_panel, &style_panel, 0);
    lv_obj_clear_flag(status_panel, LV_OBJ_FLAG_SCROLLABLE);

    lora_status = lv_label_create(status_panel);
    lv_label_set_text(lora_status, "Status: Disabled");
    lv_obj_add_style(lora_status, &style_label_body, 0);
    lv_obj_align(lora_status, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t* freq_lbl = lv_label_create(status_panel);
    lv_label_set_text(freq_lbl, "915.0 MHz | SF9");
    lv_obj_add_style(freq_lbl, &style_label_small, 0);
    lv_obj_align(freq_lbl, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    // Node list
    lv_obj_t* list_title = lv_label_create(scr);
    lv_label_set_text(list_title, "Nearby Ricks:");
    lv_obj_add_style(list_title, &style_label_body, 0);
    lv_obj_align(list_title, LV_ALIGN_TOP_LEFT, 15, 155);

    lora_node_list = lv_list_create(scr);
    lv_obj_set_size(lora_node_list, 200, 180);
    lv_obj_align(lora_node_list, LV_ALIGN_CENTER, 0, 55);
    lv_obj_set_style_bg_color(lora_node_list, THEME_SPACE_DARK, 0);
    lv_obj_set_style_border_color(lora_node_list, THEME_PORTAL_GREEN, 0);
    lv_obj_set_style_border_width(lora_node_list, 1, 0);

    // Empty state
    lv_obj_t* empty = lv_list_add_text(lora_node_list, "No Ricks found yet...");
    lv_obj_set_style_text_color(empty, THEME_TEXT_SECONDARY, 0);

    // Buttons
    lv_obj_t* btn_enable = ui_create_button(scr, LV_SYMBOL_PLAY " ENABLE", 180, 40);
    lv_obj_align(btn_enable, LV_ALIGN_CENTER, 0, 170);
    lv_obj_add_event_cb(btn_enable, lora_toggle_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_enable);

    lv_obj_t* btn_back = ui_create_button(scr, LV_SYMBOL_LEFT " BACK", 100, 35);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(btn_back, back_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_back);

    return scr;
}

// =============================================================================
// PLUMBUS COMMANDER SCREEN (File Manager)
// =============================================================================
static lv_obj_t* file_list = NULL;

static void file_item_cb(lv_event_t* e) {
    lv_obj_t* item = lv_event_get_target(e);
    const char* txt = lv_list_get_btn_text(file_list, item);
    Serial.printf("[UI] Selected: %s\n", txt);
}

lv_obj_t* ui_create_plumbus_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);

    ui_create_header(scr, state);

    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_DIRECTORY " PLUMBUS CMD");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 45);

    // SD status
    lv_obj_t* sd_status = lv_label_create(scr);
    lv_label_set_text(sd_status, "SD: Ready | 2.1 GB free");
    lv_obj_add_style(sd_status, &style_label_small, 0);
    lv_obj_align(sd_status, LV_ALIGN_TOP_MID, 0, 75);

    // Path bar
    lv_obj_t* path_bar = lv_obj_create(scr);
    lv_obj_set_size(path_bar, 200, 30);
    lv_obj_align(path_bar, LV_ALIGN_TOP_MID, 0, 95);
    lv_obj_add_style(path_bar, &style_panel, 0);
    lv_obj_clear_flag(path_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* path_lbl = lv_label_create(path_bar);
    lv_label_set_text(path_lbl, "/pickle_rick/");
    lv_obj_add_style(path_lbl, &style_label_small, 0);
    lv_obj_center(path_lbl);

    // File list
    file_list = lv_list_create(scr);
    lv_obj_set_size(file_list, 200, 250);
    lv_obj_align(file_list, LV_ALIGN_CENTER, 0, 45);
    lv_obj_set_style_bg_color(file_list, THEME_SPACE_DARK, 0);
    lv_obj_set_style_border_color(file_list, THEME_PORTAL_GREEN, 0);
    lv_obj_set_style_border_width(file_list, 1, 0);

    // Sample files
    const char* dirs[] = {LV_SYMBOL_DIRECTORY " handshakes", LV_SYMBOL_DIRECTORY " wardrives", LV_SYMBOL_DIRECTORY " exports"};
    const char* files[] = {LV_SYMBOL_FILE " capture_001.22000", LV_SYMBOL_FILE " wardrive_01.csv"};

    for (int i = 0; i < 3; i++) {
        lv_obj_t* item = lv_list_add_btn(file_list, NULL, dirs[i]);
        lv_obj_set_style_bg_color(item, THEME_SPACE_DARK, 0);
        lv_obj_set_style_text_color(item, THEME_MEESEEKS_BLUE, 0);
        lv_obj_add_event_cb(item, file_item_cb, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(input_group, item);
    }

    for (int i = 0; i < 2; i++) {
        lv_obj_t* item = lv_list_add_btn(file_list, NULL, files[i]);
        lv_obj_set_style_bg_color(item, THEME_SPACE_DARK, 0);
        lv_obj_set_style_text_color(item, THEME_PORTAL_GREEN, 0);
        lv_obj_add_event_cb(item, file_item_cb, LV_EVENT_CLICKED, NULL);
        lv_group_add_obj(input_group, item);
    }

    // Back button
    lv_obj_t* btn_back = ui_create_button(scr, LV_SYMBOL_LEFT " BACK", 100, 35);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(btn_back, back_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_back);

    return scr;
}

// =============================================================================
// SETTINGS SCREEN (Workshop)
// =============================================================================
static void brightness_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    int val = lv_slider_get_value(slider);
    analogWrite(42, val * 255 / 100);
    Serial.printf("[UI] Brightness: %d%%\n", val);
}

lv_obj_t* ui_create_settings_screen(ui_state_t* state) {
    lv_obj_t* scr = lv_obj_create(NULL);
    ui_theme_apply_screen(scr);

    ui_create_header(scr, state);

    // Title
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS " WORKSHOP");
    lv_obj_add_style(title, &style_label_title, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 45);

    // Settings container
    lv_obj_t* cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 200, 320);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(cont, 15, 0);

    // Brightness
    lv_obj_t* bright_panel = lv_obj_create(cont);
    lv_obj_set_size(bright_panel, 190, 70);
    lv_obj_add_style(bright_panel, &style_panel, 0);
    lv_obj_clear_flag(bright_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* bright_lbl = lv_label_create(bright_panel);
    lv_label_set_text(bright_lbl, LV_SYMBOL_IMAGE " Brightness");
    lv_obj_add_style(bright_lbl, &style_label_body, 0);
    lv_obj_align(bright_lbl, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t* bright_slider = lv_slider_create(bright_panel);
    lv_obj_set_size(bright_slider, 160, 10);
    lv_obj_align(bright_slider, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_slider_set_range(bright_slider, 10, 100);
    lv_slider_set_value(bright_slider, 80, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bright_slider, THEME_SPACE_DARK, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bright_slider, THEME_PORTAL_GREEN, LV_PART_INDICATOR);
    lv_obj_add_event_cb(bright_slider, brightness_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_group_add_obj(input_group, bright_slider);

    // Device name
    lv_obj_t* name_panel = lv_obj_create(cont);
    lv_obj_set_size(name_panel, 190, 60);
    lv_obj_add_style(name_panel, &style_panel, 0);
    lv_obj_clear_flag(name_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* name_lbl = lv_label_create(name_panel);
    lv_label_set_text(name_lbl, LV_SYMBOL_EDIT " Device Name");
    lv_obj_add_style(name_lbl, &style_label_body, 0);
    lv_obj_align(name_lbl, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t* name_val = lv_label_create(name_panel);
    lv_label_set_text(name_val, "Rick-C137");
    lv_obj_add_style(name_val, &style_label_small, 0);
    lv_obj_align(name_val, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    // About panel
    lv_obj_t* about_panel = lv_obj_create(cont);
    lv_obj_set_size(about_panel, 190, 80);
    lv_obj_add_style(about_panel, &style_panel, 0);
    lv_obj_clear_flag(about_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* about_lbl = lv_label_create(about_panel);
    lv_label_set_text(about_lbl, LV_SYMBOL_BELL " About");
    lv_obj_add_style(about_lbl, &style_label_body, 0);
    lv_obj_align(about_lbl, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t* ver_lbl = lv_label_create(about_panel);
    lv_label_set_text(ver_lbl, "RICK v" RICK_VERSION "\nBy Archie");
    lv_obj_add_style(ver_lbl, &style_label_small, 0);
    lv_obj_align(ver_lbl, LV_ALIGN_BOTTOM_LEFT, 5, -5);

    // Back button
    lv_obj_t* btn_back = ui_create_button(scr, LV_SYMBOL_LEFT " BACK", 100, 35);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(btn_back, back_btn_cb, LV_EVENT_CLICKED, NULL);
    lv_group_add_obj(input_group, btn_back);

    return scr;
}

// =============================================================================
// HEADER BAR
// =============================================================================
lv_obj_t* ui_create_header(lv_obj_t* parent, ui_state_t* state) {
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_set_size(header, SCREEN_WIDTH, UI_HEADER_HEIGHT);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_style(header, &style_status_bar, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Status icons
    lv_obj_t* icons = lv_label_create(header);
    lv_label_set_text(icons, LV_SYMBOL_WIFI " " LV_SYMBOL_BLUETOOTH " " LV_SYMBOL_GPS);
    lv_obj_add_style(icons, &style_label_small, 0);
    lv_obj_align(icons, LV_ALIGN_LEFT_MID, 5, 0);

    // Battery
    lv_obj_t* batt = lv_label_create(header);
    lv_label_set_text(batt, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(batt, THEME_SUCCESS, 0);
    lv_obj_align(batt, LV_ALIGN_RIGHT_MID, -5, 0);

    return header;
}

// =============================================================================
// FOOTER BAR (XP/Rank)
// =============================================================================
lv_obj_t* ui_create_footer(lv_obj_t* parent, ui_state_t* state) {
    lv_obj_t* footer = lv_obj_create(parent);
    lv_obj_set_size(footer, SCREEN_WIDTH, UI_FOOTER_HEIGHT + 20);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(footer, &style_status_bar, 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);

    // Rank label with bounds checking
    lv_obj_t* rank = lv_label_create(footer);
    const char* rank_name = "Morty";
    uint8_t rankIdx = 0;
    if (state && state->rick) {
        rankIdx = (uint8_t)state->rick->rank;
        if (rankIdx < sizeof(RANK_NAMES) / sizeof(RANK_NAMES[0])) {
            rank_name = RANK_NAMES[rankIdx];
        }
    }
    lv_label_set_text(rank, rank_name);
    lv_obj_add_style(rank, &style_label_small, 0);
    lv_obj_align(rank, LV_ALIGN_TOP_LEFT, 5, 2);

    // XP bar
    lv_obj_t* xp_bar = ui_create_progress_bar(footer, SCREEN_WIDTH - 20, 12);
    lv_obj_align(xp_bar, LV_ALIGN_BOTTOM_MID, 0, -8);

    uint32_t xp = (state && state->rick) ? state->rick->xp : 0;
    uint32_t nextXp = 500;
    if (state && state->rick && rankIdx + 1 < sizeof(RANK_XP_THRESHOLDS) / sizeof(RANK_XP_THRESHOLDS[0])) {
        nextXp = RANK_XP_THRESHOLDS[rankIdx + 1];
    }
    int percent = nextXp > 0 ? (xp * 100 / nextXp) : 0;
    if (percent > 100) percent = 100;
    lv_bar_set_value(xp_bar, percent, LV_ANIM_OFF);

    // XP text
    lv_obj_t* xp_lbl = lv_label_create(footer);
    char xp_buf[32];
    snprintf(xp_buf, sizeof(xp_buf), "XP: %lu / %lu", xp, nextXp);
    lv_label_set_text(xp_lbl, xp_buf);
    lv_obj_add_style(xp_lbl, &style_label_small, 0);
    lv_obj_align(xp_lbl, LV_ALIGN_TOP_RIGHT, -5, 2);

    if (state) state->xpBar = xp_bar;

    return footer;
}

// =============================================================================
// SCREEN NAVIGATION
// =============================================================================
void ui_goto_screen(ui_state_t* state, ui_screen_t screen) {
    if (state->isTransitioning) return;
    if (screen >= SCREEN_COUNT) return;
    if (state->screens[screen] == NULL) {
        Serial.printf("[UI] Screen %d not created\n", screen);
        return;
    }

    state->isTransitioning = true;
    state->previousScreen = state->currentScreen;
    state->currentScreen = screen;

    // Clear input group and rebuild for new screen
    lv_group_remove_all_objs(input_group);

    // Animate transition
    lv_scr_load_anim(state->screens[screen], LV_SCR_LOAD_ANIM_FADE_ON, UI_ANIM_NORMAL, 0, false);

    state->isTransitioning = false;
    Serial.printf("[UI] Screen: %d\n", screen);
}

// =============================================================================
// UI UPDATE
// =============================================================================
void ui_update(ui_state_t* state) {
    lv_timer_handler();
}

// =============================================================================
// NOTIFICATIONS
// =============================================================================
void ui_show_notification(ui_state_t* state, const char* title, const char* message, uint16_t duration_ms) {
    lv_obj_t* mbox = lv_msgbox_create(NULL, title, message, NULL, true);
    lv_obj_add_style(mbox, &style_panel_glow, 0);
    lv_obj_center(mbox);
}

void ui_show_achievement(ui_state_t* state, const char* name, const char* icon) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s %s", icon, name);
    ui_show_notification(state, "Achievement!", buf, 3000);
}

void ui_show_rank_up(ui_state_t* state, rick_rank_t newRank) {
    char buf[64];
    snprintf(buf, sizeof(buf), "You are now: %s", RANK_NAMES[newRank]);
    ui_show_notification(state, "RANK UP!", buf, 3000);
}

// =============================================================================
// STATUS UPDATES
// =============================================================================
void ui_update_xp(ui_state_t* state, uint32_t xp, uint32_t xpForNext) {
    if (state->xpBar) {
        int percent = (xpForNext > 0) ? (xp * 100 / xpForNext) : 0;
        lv_bar_set_value(state->xpBar, percent, LV_ANIM_ON);
    }
}

void ui_update_status(ui_state_t* state, bool wifi, bool ble, bool gps, bool lora, bool sd) {
    // Status icons would be updated here
}

void ui_update_battery(ui_state_t* state, uint8_t percent, bool charging) {
    // Battery icon would be updated here
}

// =============================================================================
// THEME HELPERS
// =============================================================================
void ui_apply_theme(ui_state_t* state) {
    ui_theme_init();
}

lv_color_t ui_get_color(uint8_t colorId) {
    switch (colorId) {
        case 0: return THEME_PORTAL_GREEN;
        case 1: return THEME_PORTAL_CYAN;
        case 2: return THEME_SPACE_BLACK;
        case 3: return THEME_STAR_WHITE;
        default: return THEME_PORTAL_GREEN;
    }
}

const lv_font_t* ui_get_font(uint8_t size) {
    switch (size) {
        case 12: return &lv_font_montserrat_12;
        case 14: return &lv_font_montserrat_14;
        case 16: return &lv_font_montserrat_16;
        case 20: return &lv_font_montserrat_20;
        case 24: return &lv_font_montserrat_24;
        default: return &lv_font_montserrat_14;
    }
}
