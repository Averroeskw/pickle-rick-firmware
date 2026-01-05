/**
 * @file wardriving.cpp
 * @brief GPS Wardriving - Wubba Lubba Dub Dub Mode
 *
 * "Wubba lubba dub dub!" (I am in great pain, please help me wardrive)
 */

#include "wardriving.h"
#include <SD.h>
#include <math.h>

// Earth radius in meters
#define EARTH_RADIUS 6371000.0

// =============================================================================
// INITIALIZATION
// =============================================================================
bool wardrive_init(wardrive_state_t* state, uint32_t max_points) {
    state->points = (wardrive_point_t*)ps_malloc(sizeof(wardrive_point_t) * max_points);
    if (!state->points) {
        Serial.println("[WARDRIVE] Failed to allocate buffer");
        return false;
    }

    state->pointCount = 0;
    state->capacity = max_points;
    state->isActive = false;
    state->startTime = 0;
    state->totalDistance = 0;

    memset(&state->lastFix, 0, sizeof(gps_fix_t));
    state->sessionFile[0] = '\0';

    Serial.println("[WARDRIVE] 🚗 Wubba Lubba Dub Dub mode initialized");
    return true;
}

// =============================================================================
// SESSION CONTROL
// =============================================================================
void wardrive_start(wardrive_state_t* state) {
    state->isActive = true;
    state->startTime = millis();
    state->totalDistance = 0;
    state->pointCount = 0;

    // Create session filename with timestamp
    snprintf(state->sessionFile, sizeof(state->sessionFile),
             "%s/wardrive_%lu.csv", DIR_WARDRIVING, millis() / 1000);

    // Write CSV header
    File file = SD.open(state->sessionFile, FILE_WRITE);
    if (file) {
        file.println(WIGLE_CSV_HEADER);
        file.close();
    }

    Serial.println("[WARDRIVE] 🚗 WUBBA LUBBA DUB DUB! Session started");
    Serial.printf("[WARDRIVE] Logging to: %s\n", state->sessionFile);
}

void wardrive_stop(wardrive_state_t* state) {
    state->isActive = false;

    Serial.printf("[WARDRIVE] Session ended. Points: %d, Distance: %.2f km\n",
                  state->pointCount, state->totalDistance / 1000.0);
}

// =============================================================================
// GPS UPDATE
// =============================================================================
void wardrive_update_gps(wardrive_state_t* state, TinyGPSPlus* gps) {
    if (!gps->location.isValid()) {
        state->lastFix.valid = false;
        return;
    }

    gps_fix_t newFix;
    newFix.latitude = gps->location.lat();
    newFix.longitude = gps->location.lng();
    newFix.altitude = gps->altitude.meters();
    newFix.speed = gps->speed.kmph();
    newFix.course = gps->course.deg();
    newFix.satellites = gps->satellites.value();
    newFix.timestamp = millis();
    newFix.valid = true;

    // Calculate distance traveled
    if (state->lastFix.valid) {
        float dist = gps_distance(
            state->lastFix.latitude, state->lastFix.longitude,
            newFix.latitude, newFix.longitude
        );
        state->totalDistance += dist;
    }

    state->lastFix = newFix;
}

// =============================================================================
// NETWORK LOGGING
// =============================================================================
void wardrive_add_network(wardrive_state_t* state, network_info_t* network) {
    if (!state->isActive) return;
    if (!state->lastFix.valid) return;

    // Check if already logged (by BSSID)
    for (uint32_t i = 0; i < state->pointCount; i++) {
        if (memcmp(state->points[i].bssid, network->bssid, 6) == 0) {
            // Update RSSI if stronger
            if (network->rssi > state->points[i].rssi) {
                state->points[i].rssi = network->rssi;
                state->points[i].latitude = state->lastFix.latitude;
                state->points[i].longitude = state->lastFix.longitude;
                state->points[i].lastSeen = millis();
            }
            return;
        }
    }

    // Add new point
    if (state->pointCount < state->capacity) {
        wardrive_point_t* point = &state->points[state->pointCount];
        memcpy(point->bssid, network->bssid, 6);
        strncpy(point->ssid, network->ssid, 32);
        point->ssid[32] = '\0';
        point->rssi = network->rssi;
        point->channel = network->channel;
        point->authmode = network->authmode;
        point->latitude = state->lastFix.latitude;
        point->longitude = state->lastFix.longitude;
        point->altitude = state->lastFix.altitude;
        point->firstSeen = millis();
        point->lastSeen = millis();

        state->pointCount++;

        Serial.printf("[WARDRIVE] 📍 %s @ %.6f, %.6f (RSSI: %d)\n",
                      point->ssid[0] ? point->ssid : "<hidden>",
                      point->latitude, point->longitude, point->rssi);
    }
}

// =============================================================================
// WARDRIVING TICK
// =============================================================================
void wardrive_tick(wardrive_state_t* state, scanner_state_t* scanner, TinyGPSPlus* gps) {
    if (!state->isActive) return;

    // Update GPS
    wardrive_update_gps(state, gps);

    // Log new networks
    for (uint16_t i = 0; i < scanner->count; i++) {
        wardrive_add_network(state, &scanner->networks[i]);
    }

    // Auto-save every 100 points
    if (state->pointCount > 0 && state->pointCount % 100 == 0) {
        wardrive_save(state);
    }
}

// =============================================================================
// FILE EXPORT
// =============================================================================
bool wardrive_save(wardrive_state_t* state) {
    if (state->sessionFile[0] == '\0') return false;

    File file = SD.open(state->sessionFile, FILE_APPEND);
    if (!file) return false;

    for (uint32_t i = 0; i < state->pointCount; i++) {
        wardrive_point_t* p = &state->points[i];

        // Auth mode string
        const char* auth;
        switch (p->authmode) {
            case WIFI_AUTH_OPEN:           auth = "[OPEN]"; break;
            case WIFI_AUTH_WEP:            auth = "[WEP]"; break;
            case WIFI_AUTH_WPA_PSK:        auth = "[WPA-PSK]"; break;
            case WIFI_AUTH_WPA2_PSK:       auth = "[WPA2-PSK]"; break;
            case WIFI_AUTH_WPA_WPA2_PSK:   auth = "[WPA-WPA2-PSK]"; break;
            case WIFI_AUTH_WPA2_ENTERPRISE:auth = "[WPA2-EAP]"; break;
            case WIFI_AUTH_WPA3_PSK:       auth = "[WPA3-PSK]"; break;
            default:                       auth = "[UNKNOWN]"; break;
        }

        // WiGLE CSV format
        file.printf("%02X:%02X:%02X:%02X:%02X:%02X,%s,%s,%lu,%d,%d,%.8f,%.8f,%.1f,10,WIFI\n",
                    p->bssid[0], p->bssid[1], p->bssid[2],
                    p->bssid[3], p->bssid[4], p->bssid[5],
                    p->ssid, auth, p->firstSeen,
                    p->channel, p->rssi,
                    p->latitude, p->longitude, p->altitude);
    }

    file.close();
    Serial.printf("[WARDRIVE] Saved %d points to %s\n", state->pointCount, state->sessionFile);
    return true;
}

bool wardrive_export_wigle(wardrive_state_t* state, const char* filename) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return false;

    // WiGLE header
    file.println("WigleWifi-1.4,appRelease=PickleRick,model=K257,release=1.0,device=AVERROES,display=ST7796,board=ESP32S3,brand=LilyGo");
    file.println(WIGLE_CSV_HEADER);

    for (uint32_t i = 0; i < state->pointCount; i++) {
        wardrive_point_t* p = &state->points[i];

        const char* auth;
        switch (p->authmode) {
            case WIFI_AUTH_OPEN:           auth = "[OPEN]"; break;
            case WIFI_AUTH_WEP:            auth = "[WEP]"; break;
            case WIFI_AUTH_WPA_PSK:        auth = "[WPA-PSK]"; break;
            case WIFI_AUTH_WPA2_PSK:       auth = "[WPA2-PSK]"; break;
            default:                       auth = "[WPA2]"; break;
        }

        file.printf("%02X:%02X:%02X:%02X:%02X:%02X,%s,%s,%lu,%d,%d,%.8f,%.8f,%.1f,10,WIFI\n",
                    p->bssid[0], p->bssid[1], p->bssid[2],
                    p->bssid[3], p->bssid[4], p->bssid[5],
                    p->ssid, auth, p->firstSeen,
                    p->channel, p->rssi,
                    p->latitude, p->longitude, p->altitude);
    }

    file.close();
    return true;
}

bool wardrive_export_kml(wardrive_state_t* state, const char* filename) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return false;

    // KML header
    file.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    file.println("<kml xmlns=\"http://www.opengis.net/kml/2.2\">");
    file.println("<Document>");
    file.println("<name>Pickle Rick Wardriving</name>");

    // Style for networks
    file.println("<Style id=\"network\"><IconStyle><Icon><href>http://maps.google.com/mapfiles/kml/paddle/wht-blank.png</href></Icon></IconStyle></Style>");

    // Add placemarks
    for (uint32_t i = 0; i < state->pointCount; i++) {
        wardrive_point_t* p = &state->points[i];

        file.println("<Placemark>");
        file.printf("<name>%s</name>\n", p->ssid[0] ? p->ssid : "Hidden");
        file.printf("<description>BSSID: %02X:%02X:%02X:%02X:%02X:%02X, CH: %d, RSSI: %d</description>\n",
                    p->bssid[0], p->bssid[1], p->bssid[2],
                    p->bssid[3], p->bssid[4], p->bssid[5],
                    p->channel, p->rssi);
        file.println("<styleUrl>#network</styleUrl>");
        file.printf("<Point><coordinates>%.8f,%.8f,%.1f</coordinates></Point>\n",
                    p->longitude, p->latitude, p->altitude);
        file.println("</Placemark>");
    }

    file.println("</Document>");
    file.println("</kml>");
    file.close();

    return true;
}

// =============================================================================
// STATISTICS
// =============================================================================
float wardrive_get_distance(wardrive_state_t* state) {
    return state->totalDistance / 1000.0;  // Return in km
}

uint32_t wardrive_get_duration(wardrive_state_t* state) {
    if (!state->isActive || state->startTime == 0) return 0;
    return (millis() - state->startTime) / 1000;
}

uint32_t wardrive_get_unique_count(wardrive_state_t* state) {
    return state->pointCount;
}

// =============================================================================
// GPS HELPERS
// =============================================================================
float gps_distance(double lat1, double lon1, double lat2, double lon2) {
    // Haversine formula
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = sin(dLat / 2) * sin(dLat / 2) +
               sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS * c;  // Returns meters
}

bool gps_has_fix(TinyGPSPlus* gps) {
    return gps->location.isValid() && gps->satellites.value() >= 4;
}

float gps_get_accuracy(TinyGPSPlus* gps) {
    // Estimate based on satellite count
    int sats = gps->satellites.value();
    if (sats >= 12) return 2.5;
    if (sats >= 8) return 5.0;
    if (sats >= 6) return 10.0;
    if (sats >= 4) return 25.0;
    return 100.0;
}

void gps_format_coords(double lat, double lon, char* buffer, size_t len) {
    char ns = lat >= 0 ? 'N' : 'S';
    char ew = lon >= 0 ? 'E' : 'W';
    snprintf(buffer, len, "%.6f%c, %.6f%c", fabs(lat), ns, fabs(lon), ew);
}

const char* gps_get_direction(double course) {
    if (course < 22.5 || course >= 337.5) return "N";
    if (course < 67.5) return "NE";
    if (course < 112.5) return "E";
    if (course < 157.5) return "SE";
    if (course < 202.5) return "S";
    if (course < 247.5) return "SW";
    if (course < 292.5) return "W";
    return "NW";
}
