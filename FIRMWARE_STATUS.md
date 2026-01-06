# RICK Firmware Status Report

## Current State: Display Working

The display is now functional using LilyGoLib. The firmware needs migration from the old TFT_eSPI/LVGL 8.x architecture to LilyGoLib/LVGL 9.x.

---

## Module Status

### Display (Working)
- **Status:** Working
- **Library:** LilyGoLib + LVGL 9.4.0
- **Notes:** Fully functional with correct colors and orientation

### WiFi Scanner (`src_backup/wifi/`)
- **Status:** Needs Testing
- **Files:** `wifi_scanner.h`, `wifi_scanner.cpp`, `handshake_capture.h`, `handshake_capture.cpp`
- **Dependencies:** ESP-IDF WiFi APIs (esp_wifi.h)
- **Migration Needed:** None - uses standard ESP-IDF APIs
- **Features:**
  - Passive/active WiFi scanning
  - Channel hopping
  - Promiscuous mode packet capture
  - WiGLE export format
  - MAC randomization

### BLE Spam (`src_backup/ble/`)
- **Status:** Needs Testing
- **Files:** `ble_spam.h`, `ble_spam.cpp`
- **Dependencies:** NimBLE-Arduino
- **Migration Needed:** None - NimBLE is included in lib_deps
- **Features:**
  - Apple AirPods/AirDrop spam
  - Google Fast Pair spam
  - Samsung SmartTag spam
  - Windows Swift Pair spam

### LoRa Mesh (`src_backup/lora/`)
- **Status:** Needs Review
- **Files:** `lora_mesh.h`, `lora_mesh.cpp`
- **Dependencies:** RadioLib
- **Migration Needed:** Pin definitions may conflict with LilyGoLib
- **Recommendation:** Use LilyGoLib's built-in LoRa support instead (`instance.radio`)
- **Features:**
  - Mesh networking between devices
  - Beacon broadcasting
  - Handshake sharing
  - Chat messaging

### GPS Wardriving (`src_backup/gps/`)
- **Status:** Needs Migration
- **Files:** `wardriving.h`, `wardriving.cpp`
- **Dependencies:** TinyGPSPlus
- **Migration Needed:** Use `instance.gps` from LilyGoLib instead of direct TinyGPSPlus
- **Features:**
  - GPS coordinate logging
  - Network geolocation
  - WiGLE format export

### UI Manager (`src_backup/ui/`)
- **Status:** Needs Major Migration
- **Files:** `ui_manager.h`, `ui_manager.cpp`, `ui_theme.h`, `ui_theme.cpp`
- **Dependencies:** LVGL
- **Migration Needed:** LVGL 8.x → 9.x API changes
- **Key API Changes:**
  - `lv_obj_set_style_*` signature changes
  - Screen management changes
  - Animation API changes
  - Font handling changes
- **Features:**
  - Rick & Morty space theme
  - Multiple screens (Portal Gun, Get Schwifty, etc.)
  - XP bar and achievements
  - Animated starfield background

### Core System (`src_backup/core/`)
- **Status:** Needs Review
- **Files:** `pickle_rick.h`, `pickle_rick.cpp`, `xp_system.h`, `xp_system.cpp`
- **Dependencies:** None specific
- **Migration Needed:** Minor - rename references if any
- **Features:**
  - Rick avatar states
  - XP/leveling system
  - Achievement tracking
  - Statistics

### Mode Manager (`src_backup/modes/`)
- **Status:** Needs Review
- **Files:** `mode_manager.h`, `mode_manager.cpp`
- **Dependencies:** All other modules
- **Migration Needed:** Update to use LilyGoLib peripherals

---

## LilyGoLib Peripheral Access

With LilyGoLib, peripherals are accessed through the `instance` object:

```cpp
#include <LilyGoLib.h>
#include <LV_Helper.h>

// Display
instance.setBrightness(16);  // 1-16

// GPS
instance.gps.loop();  // Process GPS data
instance.gps.location.lat();
instance.gps.location.lng();

// LoRa
instance.radio.begin();
instance.radio.transmit(data, len);

// Haptic feedback
instance.setHapticEffects(1);
instance.vibrator();

// Keyboard
instance.keyboard.getKeyCode();

// Battery
instance.getBatteryPercent();
instance.isCharging();

// SD Card
instance.installSD();
```

---

## Migration Priority

1. **High Priority:** UI Manager (needed for any visual interface)
2. **Medium Priority:** WiFi Scanner, BLE Spam (core features)
3. **Low Priority:** LoRa Mesh, GPS Wardriving (can use sat-tracker as reference)

---

## Files to Keep

From `src_backup/`:
- `config.h` - Update with LilyGoLib compatibility
- `wifi/` - Should work as-is
- `ble/` - Should work as-is
- `core/` - Should work with minor updates

## Files to Migrate

- `ui/` - LVGL 8.x → 9.x migration required
- `lora/` - Use LilyGoLib LoRa instead
- `gps/` - Use LilyGoLib GPS instead

## Files to Remove

- `User_Setup.h` - Not needed with LilyGoLib
- `lv_conf.h` - Not needed with LilyGoLib

---

## Reference Implementation

See https://github.com/Averroeskw/k257-sat-tracker for working examples of:
- LilyGoLib initialization
- LVGL 9.x UI code
- GPS usage with LilyGoLib
- LoRa configuration
- NFC reader integration
