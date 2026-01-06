/**
 * @file xp_system.h
 * @brief XP & Gamification System
 *
 * Manages experience points, ranks, achievements, and challenges
 */

#ifndef XP_SYSTEM_H
#define XP_SYSTEM_H

#include <Arduino.h>
#include "../config.h"

// =============================================================================
// XP DATA STRUCTURES
// =============================================================================
typedef struct {
    uint32_t totalXP;
    rick_rank_t rank;
    uint32_t networksScanned;
    uint32_t handshakesCaptured;
    uint32_t pmkidsExtracted;
    uint32_t bleDevicesSpammed;
    uint32_t wardrivePoints;
    uint32_t loraMessages;
    uint32_t challengesCompleted;
    uint32_t achievementsUnlocked;
    float distanceWardriven;        // in km
    uint32_t totalScanTime;         // in seconds
    uint32_t sessionsStarted;
    uint64_t firstBootTimestamp;
} xp_stats_t;

// =============================================================================
// ACHIEVEMENT DATA
// =============================================================================
typedef struct {
    achievement_id_t id;
    const char* name;
    const char* description;
    const char* icon;
    uint32_t xpReward;
    bool unlocked;
    uint64_t unlockedAt;
} achievement_t;

// =============================================================================
// CHALLENGE DATA
// =============================================================================
typedef struct {
    challenge_type_t type;
    const char* name;
    const char* description;
    uint32_t target;
    uint32_t progress;
    uint32_t xpReward;
    bool completed;
    uint64_t expiresAt;
} challenge_t;

// =============================================================================
// XP SYSTEM FUNCTIONS
// =============================================================================

/**
 * Initialize XP system
 */
void xp_init(xp_stats_t* stats);

/**
 * Award XP with mood modifier
 */
void xp_award(xp_stats_t* stats, uint32_t amount, rick_mood_t mood);

/**
 * Check and update rank
 */
bool xp_check_rank_up(xp_stats_t* stats);

/**
 * Save XP to SD card
 */
bool xp_save(xp_stats_t* stats);

/**
 * Load XP from SD card
 */
bool xp_load(xp_stats_t* stats);

// =============================================================================
// ACHIEVEMENT FUNCTIONS
// =============================================================================

/**
 * Initialize achievements
 */
void achievements_init();

/**
 * Check if achievement condition is met
 */
bool achievement_check(achievement_id_t id, xp_stats_t* stats);

/**
 * Unlock achievement and award XP
 */
void achievement_unlock(achievement_id_t id, xp_stats_t* stats);

/**
 * Get achievement by ID
 */
achievement_t* achievement_get(achievement_id_t id);

/**
 * Get unlocked achievement count
 */
uint32_t achievements_unlocked_count();

// =============================================================================
// CHALLENGE FUNCTIONS
// =============================================================================

/**
 * Generate daily challenges
 */
void challenges_generate(challenge_t* challenges, uint8_t count);

/**
 * Update challenge progress
 */
void challenge_update(challenge_t* challenge, uint32_t progress);

/**
 * Check if challenge is completed
 */
bool challenge_check_complete(challenge_t* challenge, xp_stats_t* stats);

// =============================================================================
// ACHIEVEMENT DEFINITIONS - Using ASCII icons (emojis crash ESP32)
// =============================================================================
static const achievement_t ACHIEVEMENTS[] = {
    {ACH_WUBBA_LUBBA, "Wubba Lubba Dub Dub", "First boot completed", "[!]", 100, false, 0},
    {ACH_IM_PICKLE_RICK, "I'm Pickle Rick!", "Capture 100 handshakes", "[P]", 1000, false, 0},
    {ACH_GET_SCHWIFTY, "Get Schwifty", "BLE spam 1000 devices", "[S]", 500, false, 0},
    {ACH_PORTAL_HOPPER, "Portal Hopper", "Scan in 5 different locations", "[O]", 300, false, 0},
    {ACH_COUNCIL_OF_RICKS, "Council of Ricks", "Reach maximum rank", "[*]", 5000, false, 0},
    {ACH_PLUMBUS_MASTER, "Plumbus Master", "Manage 100 files", "[T]", 200, false, 0},
    {ACH_MEESEEKS_BOX, "Meeseeks Box", "Complete 50 challenges", "[B]", 750, false, 0},
    {ACH_MICROVERSE, "Microverse", "Use spectrum mode for 1 hour total", "[M]", 400, false, 0},
    {ACH_INTERDIMENSIONAL, "Interdimensional Cable", "Capture all auth types", "[I]", 800, false, 0},
    {ACH_SHOW_ME_WHAT_YOU_GOT, "Show Me What You Got", "Capture first handshake", "[G]", 150, false, 0},
    {ACH_TINY_RICK, "Tiny Rick!", "Use power save mode", "[+]", 100, false, 0},
    {ACH_SCARY_TERRY, "Scary Terry", "Deauth 100 clients", "[X]", 500, false, 0},
    {ACH_BIRD_PERSON, "Bird Person", "Send 100 LoRa messages", "[>]", 350, false, 0},
    {ACH_SQUANCHY, "Squanchy", "Use all modes in one session", "[#]", 600, false, 0},
    {ACH_EVIL_MORTY, "Evil Morty", "Detect a rogue access point", "[E]", 1000, false, 0},
    // Add more achievements to reach 63...
};

// =============================================================================
// RANK THRESHOLDS
// =============================================================================
static const uint32_t RANK_XP_THRESHOLDS[] = {
    0,          // MORTY
    500,        // SUMMER
    1500,       // BETH
    3000,       // JERRY
    5000,       // MR_MEESEEKS
    8000,       // SCARY_TERRY
    12000,      // BIRDPERSON
    17000,      // SQUANCHY
    23000,      // UNITY
    30000,      // KROMBOPULOS
    40000,      // EVIL_MORTY
    52000,      // PICKLE_RICK
    66000,      // TOXIC_RICK
    82000,      // TINY_RICK
    100000,     // DOOFUS_RICK
    125000,     // COUNCIL_RICK
    155000,     // RICK_PRIME
    190000,     // C137_RICK
    230000,     // PORTAL_MASTER
    275000,     // DIMENSION_HOPPER
    330000      // MULTIVERSE_GOD
};

static const char* RANK_NAMES[] = {
    "Morty",
    "Summer",
    "Beth",
    "Jerry",
    "Mr. Meeseeks",
    "Scary Terry",
    "Birdperson",
    "Squanchy",
    "Unity",
    "Krombopulos Michael",
    "Evil Morty",
    "Pickle Rick",
    "Toxic Rick",
    "Tiny Rick",
    "Doofus Rick",
    "Council Rick",
    "Rick Prime",
    "Rick C-137",
    "Portal Master",
    "Dimension Hopper",
    "Multiverse God"
};

#endif // XP_SYSTEM_H
