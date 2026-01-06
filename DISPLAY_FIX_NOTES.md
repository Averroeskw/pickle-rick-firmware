# K257 T-LoRa Pager Display Fix Notes

## Problem
Display was showing wrong colors, wrong orientation, or completely off when using TFT_eSPI directly.

## Solution
Use **LilyGoLib** library which handles all hardware abstraction internally.

## Key Configuration Requirements

### 1. Board JSON (`boards/lilygo-t-lora-pager.json`)
Must use these exact settings:
```json
{
  "build": {
    "arduino": {
      "partitions": "app3M_fat9M_16MB.csv",
      "memory_type": "qio_qspi"  // NOT qio_opi!
    },
    "variant": "lilygo_twatch_ultra",  // Uses this variant, NOT lilygo_tlora_pager
    "variants_dir": "variants"
  }
}
```

### 2. platformio.ini Structure
```ini
[env_arduino]
framework = arduino
platform = espressif32@6.10.0
board_build.partitions = ${platformio.libdeps_dir}/${platformio.default_envs}/LilyGoLib/examples/factory/partitions.csv

lib_deps =
    https://github.com/Xinyuan-LilyGO/LilyGoLib
    lvgl/lvgl @ ^9.4.0  # Must use LVGL 9.x, NOT 8.x

[env:pickle_rick]
extends = env_arduino
board = lilygo-t-lora-pager
build_flags =
    ${env_arduino.build_flags}
    -DARDUINO_T_LORA_PAGER
    -I variants/lilygo_tlora_pager
```

### 3. Required Variants
Need both variant directories:
- `variants/lilygo_tlora_pager/pins_arduino.h` - Actual pin definitions
- `variants/lilygo_twatch_ultra/pins_arduino.h` - Referenced by board JSON

### 4. Code Structure
```cpp
#include <LilyGoLib.h>
#include <LV_Helper.h>

void setup() {
    instance.begin();           // Initialize all hardware
    beginLvglHelper(instance);  // Initialize LVGL display
    instance.setBrightness(16); // Set backlight (1-16)
}

void loop() {
    lv_timer_handler();  // LVGL tick
    delay(5);
}
```

### 5. Display Specs
- Controller: ST7796
- Resolution: 480 x 222 (landscape)
- Interface: SPI
- Pins: MOSI=34, MISO=33, SCLK=35, CS=38, DC=37, BL=42

## What Did NOT Work
1. TFT_eSPI with manual User_Setup.h configuration
2. LVGL 8.x (must use 9.x with LilyGoLib)
3. `memory_type = qio_opi` in board JSON
4. Direct TFT_eSPI pin configuration via build flags

## Reference Repository
Working configuration copied from: https://github.com/Averroeskw/k257-sat-tracker
