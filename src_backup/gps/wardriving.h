/**
 * @file wardriving.h
 * @brief GPS Wardriving - Wubba Lubba Dub Dub Mode
 *
 * GPS-enabled wardriving with WiGLE export
 */

#ifndef WARDRIVING_H
#define WARDRIVING_H

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "../config.h"
#include "../wifi/wifi_scanner.h"

// =============================================================================
// WARDRIVING DATA
// =============================================================================
typedef struct {
    double latitude;
    double longitude;
    double altitude;
    double speed;
    double course;
    uint8_t satellites;
    uint32_t timestamp;
    bool valid;
} gps_fix_t;

typedef struct {
    uint8_t bssid[6];
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    uint8_t authmode;
    double latitude;
    double longitude;
    double altitude;
    uint32_t firstSeen;
    uint32_t lastSeen;
} wardrive_point_t;

typedef struct {
    wardrive_point_t* points;
    uint32_t pointCount;
    uint32_t capacity;
    bool isActive;
    gps_fix_t lastFix;
    uint32_t startTime;
    float totalDistance;
    char sessionFile[64];
} wardrive_state_t;

// =============================================================================
// WARDRIVING FUNCTIONS
// =============================================================================

/**
 * Initialize wardriving module
 */
bool wardrive_init(wardrive_state_t* state, uint32_t max_points);

/**
 * Start new wardriving session
 */
void wardrive_start(wardrive_state_t* state);

/**
 * Stop wardriving session
 */
void wardrive_stop(wardrive_state_t* state);

/**
 * Update GPS fix
 */
void wardrive_update_gps(wardrive_state_t* state, TinyGPSPlus* gps);

/**
 * Add network to wardrive log
 */
void wardrive_add_network(wardrive_state_t* state, network_info_t* network);

/**
 * Wardriving tick - call in loop
 */
void wardrive_tick(wardrive_state_t* state, scanner_state_t* scanner, TinyGPSPlus* gps);

/**
 * Save session to SD card
 */
bool wardrive_save(wardrive_state_t* state);

/**
 * Export to WiGLE CSV format
 */
bool wardrive_export_wigle(wardrive_state_t* state, const char* filename);

/**
 * Export to KML format (Google Earth)
 */
bool wardrive_export_kml(wardrive_state_t* state, const char* filename);

/**
 * Get total distance traveled (km)
 */
float wardrive_get_distance(wardrive_state_t* state);

/**
 * Get session duration (seconds)
 */
uint32_t wardrive_get_duration(wardrive_state_t* state);

/**
 * Get unique network count
 */
uint32_t wardrive_get_unique_count(wardrive_state_t* state);

/**
 * Calculate distance between two GPS points (Haversine)
 */
float gps_distance(double lat1, double lon1, double lat2, double lon2);

// =============================================================================
// GPS HELPERS
// =============================================================================

/**
 * Check if GPS has valid fix
 */
bool gps_has_fix(TinyGPSPlus* gps);

/**
 * Get GPS accuracy estimate (meters)
 */
float gps_get_accuracy(TinyGPSPlus* gps);

/**
 * Format GPS coordinates for display
 */
void gps_format_coords(double lat, double lon, char* buffer, size_t len);

/**
 * Get cardinal direction from course
 */
const char* gps_get_direction(double course);

#endif // WARDRIVING_H
