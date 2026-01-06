/**
 * @file pickle_rick.h
 * @brief Rick Avatar & Character System
 *
 * Manages Rick character state, mood, and visual representation
 * Rebranded from PORKCHOP's Piglet system
 */

#ifndef PICKLE_RICK_CORE_H
#define PICKLE_RICK_CORE_H

#include <Arduino.h>
#include "../config.h"

// =============================================================================
// RICK CHARACTER EXPRESSIONS
// =============================================================================
typedef enum {
    EXPR_NEUTRAL = 0,       // Default Rick face
    EXPR_GENIUS,            // Smart/calculating look
    EXPR_DRUNK,             // Classic drunk Rick
    EXPR_ANGRY,             // Angry eyebrows
    EXPR_BURP,              // Mid-burp
    EXPR_WINK,              // Knowing wink
    EXPR_PORTAL,            // Looking through portal
    EXPR_SCANNING,          // Focused scanning look
    EXPR_CAPTURING,         // Intense capture mode
    EXPR_SUCCESS,           // Smug success
    EXPR_BORED,             // Bored/unimpressed
    EXPR_COUNT
} rick_expression_t;

// =============================================================================
// RICK AVATAR STATES
// =============================================================================
typedef struct {
    rick_expression_t expression;
    rick_mood_t mood;
    uint32_t xp;
    rick_rank_t rank;
    bool isAnimating;
    uint32_t lastBlink;
    uint32_t lastMoodChange;
    uint8_t drinkLevel;         // 0-100, affects mood
    uint8_t geniusLevel;        // 0-100, affects XP multiplier
} rick_avatar_t;

// =============================================================================
// AVATAR FUNCTIONS
// =============================================================================

/**
 * Initialize Rick avatar with default state
 */
void rick_init(rick_avatar_t* rick);

/**
 * Update Rick's expression based on current activity
 */
void rick_set_expression(rick_avatar_t* rick, rick_expression_t expr);

/**
 * Update Rick's mood (affects buffs/debuffs)
 */
void rick_set_mood(rick_avatar_t* rick, rick_mood_t mood);

/**
 * Get XP multiplier based on mood
 */
float rick_get_xp_multiplier(rick_avatar_t* rick);

/**
 * Get scan speed multiplier based on mood
 */
float rick_get_scan_multiplier(rick_avatar_t* rick);

/**
 * Trigger random mood shift (called periodically)
 */
void rick_random_mood(rick_avatar_t* rick);

/**
 * Get quote based on current mood/expression
 */
const char* rick_get_quote(rick_avatar_t* rick);

/**
 * Update avatar animations (blink, etc.)
 */
void rick_update_animation(rick_avatar_t* rick);

/**
 * Draw Rick avatar at specified position
 * @param x X position
 * @param y Y position
 * @param size Avatar size (small/medium/large)
 */
void rick_draw(rick_avatar_t* rick, int x, int y, uint8_t size);

// =============================================================================
// RANK SYSTEM
// =============================================================================

/**
 * Get XP required for next rank
 */
uint32_t rick_xp_for_rank(rick_rank_t rank);

/**
 * Calculate rank from total XP
 */
rick_rank_t rick_calculate_rank(uint32_t xp);

/**
 * Get rank name string
 */
const char* rick_rank_name(rick_rank_t rank);

/**
 * Get rank icon/emoji
 */
const char* rick_rank_icon(rick_rank_t rank);

// =============================================================================
// QUOTES DATABASE
// =============================================================================
static const char* RICK_QUOTES[] = {
    // General
    "Wubba lubba dub dub!",
    "I'm not a hero. I'm a scientist.",
    "To live is to risk it all.",
    "Nobody exists on purpose.",
    "Existence is pain!",

    // Scanning
    "Let's see what's out there, Morty.",
    "Scanning dimensions... I mean WiFi.",
    "The multiverse is full of unsecured networks.",
    "Time to portal hop through some SSIDs.",

    // Capture success
    "Got 'em! Another handshake for the collection.",
    "PMKID extracted. Science, bitch!",
    "That's the way the news goes!",
    "And that's the waaaay the cookie crumbles.",

    // BLE spam
    "Get Schwifty! Show me what you got!",
    "Time to spam some dimensions.",
    "Flooding the airwaves, Morty!",

    // Wardriving
    "Road trip across dimensions!",
    "Wardriving through the multiverse.",
    "Every network tells a story.",

    // Mood specific
    "I need a drink...",                    // DRUNK
    "I'm a genius, obviously.",              // GENIUS
    "EVERYTHING IS TERRIBLE!",               // ANGRY
    "I just want to die...",                 // DEPRESSED
    "I'm Pickle Rick! ...wait, no.",         // PICKLE

    NULL
};

#endif // PICKLE_RICK_CORE_H
