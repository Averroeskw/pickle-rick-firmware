/**
 * @file User_Setup.h
 * @brief TFT_eSPI Configuration for K257 T-LoRa Pager
 *
 * Display: ST7796 480x222 (rotated to 222x480 portrait)
 */

#ifndef USER_SETUP_H
#define USER_SETUP_H

// Display driver
#define ST7796_DRIVER

// Display dimensions (landscape orientation)
#define TFT_WIDTH  222
#define TFT_HEIGHT 480

// K257 Display Pins (directly connected to ESP32-S3)
#define TFT_MOSI 34
#define TFT_MISO 33
#define TFT_SCLK 35
#define TFT_CS   38
#define TFT_DC   37
#define TFT_RST  -1  // Connected to reset circuit
#define TFT_BL   42  // Backlight PWM

// SPI Settings
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

// Use hardware SPI
#define USE_HSPI_PORT

// Color order
#define TFT_RGB_ORDER TFT_BGR

// Invert colors if needed
// #define TFT_INVERSION_ON
// #define TFT_INVERSION_OFF

// Load fonts
#define LOAD_GLCD   // Font 1
#define LOAD_FONT2  // Font 2
#define LOAD_FONT4  // Font 4
#define LOAD_FONT6  // Font 6
#define LOAD_FONT7  // Font 7
#define LOAD_FONT8  // Font 8
#define LOAD_GFXFF  // FreeFonts

// Smooth fonts
#define SMOOTH_FONT

#endif // USER_SETUP_H
