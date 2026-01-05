/**
 * @file xp_system.cpp
 * @brief XP & Gamification System Implementation
 */

#include "xp_system.h"
#include <SD.h>
#include <ArduinoJson.h>

// =============================================================================
// XP SYSTEM INITIALIZATION
// =============================================================================
void xp_init(xp_stats_t* stats) {
    stats->totalXP = 0;
    stats->rank = RANK_MORTY;
    stats->networksScanned = 0;
    stats->handshakesCaptured = 0;
    stats->pmkidsExtracted = 0;
    stats->bleDevicesSpammed = 0;
    stats->wardrivePoints = 0;
    stats->loraMessages = 0;
    stats->challengesCompleted = 0;
    stats->achievementsUnlocked = 0;
    stats->distanceWardriven = 0.0f;
    stats->totalScanTime = 0;
    stats->sessionsStarted = 0;
    stats->firstBootTimestamp = 0;

    Serial.println("[XP] Gamification system initialized");
}

// =============================================================================
// XP AWARD
// =============================================================================
void xp_award(xp_stats_t* stats, uint32_t amount, rick_mood_t mood) {
    float modifier = 1.0f;

    switch (mood) {
        case MOOD_GENIUS:   modifier = 1.2f; break;
        case MOOD_PICKLE:   modifier = 1.5f; break;
        case MOOD_DEPRESSED: modifier = 0.9f; break;
        case MOOD_ANGRY:    modifier = 1.0f; break;
        case MOOD_DRUNK:    modifier = 1.0f; break;
        case MOOD_WUBBA_LUBBA: modifier = 1.0f + (random(0, 50) / 100.0f); break;
        default:            modifier = 1.0f; break;
    }

    uint32_t awarded = (uint32_t)(amount * modifier);
    stats->totalXP += awarded;

    Serial.printf("[XP] +%d (modifier: %.2f) | Total: %d\n",
                  awarded, modifier, stats->totalXP);
}

// =============================================================================
// RANK CHECK
// =============================================================================
bool xp_check_rank_up(xp_stats_t* stats) {
    rick_rank_t newRank = RANK_MORTY;

    // Find highest rank achieved
    for (int i = RANK_MAX - 1; i >= 0; i--) {
        if (stats->totalXP >= RANK_XP_THRESHOLDS[i]) {
            newRank = (rick_rank_t)i;
            break;
        }
    }

    if (newRank > stats->rank) {
        stats->rank = newRank;
        return true;
    }
    return false;
}

// =============================================================================
// PERSISTENCE
// =============================================================================
bool xp_save(xp_stats_t* stats) {
    File file = SD.open("/sd/pickle_rick/xp/stats.json", FILE_WRITE);
    if (!file) {
        Serial.println("[XP] Failed to save stats");
        return false;
    }

    JsonDocument doc;
    doc["totalXP"] = stats->totalXP;
    doc["rank"] = stats->rank;
    doc["networksScanned"] = stats->networksScanned;
    doc["handshakesCaptured"] = stats->handshakesCaptured;
    doc["pmkidsExtracted"] = stats->pmkidsExtracted;
    doc["bleDevicesSpammed"] = stats->bleDevicesSpammed;
    doc["wardrivePoints"] = stats->wardrivePoints;
    doc["loraMessages"] = stats->loraMessages;
    doc["challengesCompleted"] = stats->challengesCompleted;
    doc["achievementsUnlocked"] = stats->achievementsUnlocked;
    doc["distanceWardriven"] = stats->distanceWardriven;
    doc["totalScanTime"] = stats->totalScanTime;
    doc["sessionsStarted"] = stats->sessionsStarted;
    doc["firstBootTimestamp"] = stats->firstBootTimestamp;

    serializeJson(doc, file);
    file.close();

    Serial.println("[XP] Stats saved to SD card");
    return true;
}

bool xp_load(xp_stats_t* stats) {
    File file = SD.open("/sd/pickle_rick/xp/stats.json", FILE_READ);
    if (!file) {
        Serial.println("[XP] No saved stats found, starting fresh");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("[XP] Failed to parse stats file");
        return false;
    }

    stats->totalXP = doc["totalXP"] | 0;
    stats->rank = (rick_rank_t)(doc["rank"] | 0);
    stats->networksScanned = doc["networksScanned"] | 0;
    stats->handshakesCaptured = doc["handshakesCaptured"] | 0;
    stats->pmkidsExtracted = doc["pmkidsExtracted"] | 0;
    stats->bleDevicesSpammed = doc["bleDevicesSpammed"] | 0;
    stats->wardrivePoints = doc["wardrivePoints"] | 0;
    stats->loraMessages = doc["loraMessages"] | 0;
    stats->challengesCompleted = doc["challengesCompleted"] | 0;
    stats->achievementsUnlocked = doc["achievementsUnlocked"] | 0;
    stats->distanceWardriven = doc["distanceWardriven"] | 0.0f;
    stats->totalScanTime = doc["totalScanTime"] | 0;
    stats->sessionsStarted = doc["sessionsStarted"] | 0;
    stats->firstBootTimestamp = doc["firstBootTimestamp"] | 0;

    Serial.printf("[XP] Loaded stats: %d XP, Rank %d\n", stats->totalXP, stats->rank);
    return true;
}

// =============================================================================
// ACHIEVEMENTS
// =============================================================================
static bool achievementUnlocked[ACHIEVEMENT_COUNT] = {false};

void achievements_init() {
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++) {
        achievementUnlocked[i] = false;
    }
}

bool achievement_check(achievement_id_t id, xp_stats_t* stats) {
    if (achievementUnlocked[id]) return false;

    bool unlocked = false;

    switch (id) {
        case ACH_WUBBA_LUBBA:
            unlocked = (stats->sessionsStarted >= 1);
            break;
        case ACH_IM_PICKLE_RICK:
            unlocked = (stats->handshakesCaptured >= 100);
            break;
        case ACH_GET_SCHWIFTY:
            unlocked = (stats->bleDevicesSpammed >= 1000);
            break;
        case ACH_SHOW_ME_WHAT_YOU_GOT:
            unlocked = (stats->handshakesCaptured >= 1);
            break;
        case ACH_SCARY_TERRY:
            // Deauth achievement - tracked separately
            break;
        case ACH_BIRD_PERSON:
            unlocked = (stats->loraMessages >= 100);
            break;
        default:
            break;
    }

    return unlocked;
}

void achievement_unlock(achievement_id_t id, xp_stats_t* stats) {
    if (id >= ACH_COUNT || achievementUnlocked[id]) return;

    achievementUnlocked[id] = true;
    stats->achievementsUnlocked++;

    Serial.printf("\n[ACHIEVEMENT] 🏆 %s\n", ACHIEVEMENTS[id].name);
    Serial.printf("[ACHIEVEMENT] %s\n", ACHIEVEMENTS[id].description);
    Serial.printf("[ACHIEVEMENT] +%d XP\n", ACHIEVEMENTS[id].xpReward);

    stats->totalXP += ACHIEVEMENTS[id].xpReward;
}

achievement_t* achievement_get(achievement_id_t id) {
    if (id >= ACH_COUNT) return nullptr;
    return (achievement_t*)&ACHIEVEMENTS[id];
}

uint32_t achievements_unlocked_count() {
    uint32_t count = 0;
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++) {
        if (achievementUnlocked[i]) count++;
    }
    return count;
}

// =============================================================================
// CHALLENGES
// =============================================================================
void challenges_generate(challenge_t* challenges, uint8_t count) {
    static const char* challengeNames[] = {
        "Portal Hunter",
        "Handshake Collector",
        "Road Warrior",
        "Schwifty Spammer",
        "Hidden Seeker",
        "PMKID Extractor",
        "LoRa Messenger",
        "Portal Survivor",
        "Schwifty Master"
    };

    static const char* challengeDescs[] = {
        "Scan 50 networks",
        "Capture 3 handshakes",
        "Wardrive 1km",
        "Spam 100 BLE devices",
        "Find 5 hidden APs",
        "Extract 3 PMKIDs",
        "Send 10 LoRa messages",
        "Run Portal mode for 10 mins",
        "Use all BLE spam types"
    };

    static const uint32_t challengeTargets[] = {
        50, 3, 1000, 100, 5, 3, 10, 600, 4
    };

    static const uint32_t challengeXP[] = {
        100, 200, 150, 100, 250, 200, 100, 150, 300
    };

    for (uint8_t i = 0; i < count && i < CHALLENGE_COUNT; i++) {
        int randChallenge = random(0, CHALLENGE_COUNT);
        challenges[i].type = (challenge_type_t)randChallenge;
        challenges[i].name = challengeNames[randChallenge];
        challenges[i].description = challengeDescs[randChallenge];
        challenges[i].target = challengeTargets[randChallenge];
        challenges[i].progress = 0;
        challenges[i].xpReward = challengeXP[randChallenge];
        challenges[i].completed = false;
        challenges[i].expiresAt = millis() + (24 * 60 * 60 * 1000);  // 24 hours
    }
}

void challenge_update(challenge_t* challenge, uint32_t progress) {
    if (challenge->completed) return;
    challenge->progress = progress;
}

bool challenge_check_complete(challenge_t* challenge, xp_stats_t* stats) {
    if (challenge->completed) return false;

    if (challenge->progress >= challenge->target) {
        challenge->completed = true;
        stats->challengesCompleted++;
        stats->totalXP += challenge->xpReward;

        Serial.printf("\n[CHALLENGE] ✅ %s completed!\n", challenge->name);
        Serial.printf("[CHALLENGE] +%d XP\n", challenge->xpReward);

        return true;
    }
    return false;
}
