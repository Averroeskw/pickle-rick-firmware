/**
 * @file pickle_rick.cpp
 * @brief Rick Avatar & Character System Implementation
 */

#include "pickle_rick.h"
#include "xp_system.h"
#include <stdlib.h>

// =============================================================================
// AVATAR INITIALIZATION
// =============================================================================
void rick_init(rick_avatar_t* rick) {
    rick->expression = EXPR_NEUTRAL;
    rick->mood = MOOD_DRUNK;  // Classic Rick
    rick->xp = 0;
    rick->rank = RANK_MORTY;
    rick->isAnimating = false;
    rick->lastBlink = 0;
    rick->lastMoodChange = millis();
    rick->drinkLevel = 50;
    rick->geniusLevel = 80;
}

// =============================================================================
// EXPRESSION SYSTEM
// =============================================================================
void rick_set_expression(rick_avatar_t* rick, rick_expression_t expr) {
    if (rick->expression != expr) {
        rick->expression = expr;
        rick->isAnimating = true;
    }
}

// =============================================================================
// MOOD SYSTEM
// =============================================================================
void rick_set_mood(rick_avatar_t* rick, rick_mood_t mood) {
    if (rick->mood != mood) {
        rick->mood = mood;
        rick->lastMoodChange = millis();

        // Set expression based on mood
        switch (mood) {
            case MOOD_GENIUS:
                rick_set_expression(rick, EXPR_GENIUS);
                break;
            case MOOD_DRUNK:
                rick_set_expression(rick, EXPR_DRUNK);
                break;
            case MOOD_ANGRY:
                rick_set_expression(rick, EXPR_ANGRY);
                break;
            case MOOD_DEPRESSED:
                rick_set_expression(rick, EXPR_BORED);
                break;
            case MOOD_PICKLE:
                rick_set_expression(rick, EXPR_SUCCESS);
                break;
            default:
                rick_set_expression(rick, EXPR_NEUTRAL);
                break;
        }
    }
}

float rick_get_xp_multiplier(rick_avatar_t* rick) {
    switch (rick->mood) {
        case MOOD_GENIUS:   return 1.2f;
        case MOOD_PICKLE:   return 1.5f;
        case MOOD_DEPRESSED: return 0.9f;
        case MOOD_ANGRY:    return 1.0f;
        case MOOD_DRUNK:    return 1.0f;
        case MOOD_WUBBA_LUBBA: return 1.0f + (random(0, 50) / 100.0f);
        default:            return 1.0f;
    }
}

float rick_get_scan_multiplier(rick_avatar_t* rick) {
    switch (rick->mood) {
        case MOOD_ANGRY:    return 1.1f;
        case MOOD_GENIUS:   return 1.05f;
        case MOOD_DEPRESSED: return 0.9f;
        default:            return 1.0f;
    }
}

void rick_random_mood(rick_avatar_t* rick) {
    // Random mood change every 5-15 minutes
    if (millis() - rick->lastMoodChange > (5 * 60 * 1000 + random(0, 10 * 60 * 1000))) {
        rick_mood_t newMood = (rick_mood_t)random(0, 6);
        rick_set_mood(rick, newMood);
    }
}

// =============================================================================
// QUOTES
// =============================================================================
const char* rick_get_quote(rick_avatar_t* rick) {
    int quoteCount = 0;
    while (RICK_QUOTES[quoteCount] != NULL) quoteCount++;

    // Mood-specific quote ranges
    int start = 0;
    int end = quoteCount;

    switch (rick->mood) {
        case MOOD_DRUNK:
            return RICK_QUOTES[random(0, 5)];  // General
        case MOOD_GENIUS:
            return "I'm a genius, obviously.";
        case MOOD_ANGRY:
            return "EVERYTHING IS TERRIBLE!";
        case MOOD_DEPRESSED:
            return "Wubba lubba dub dub...";
        case MOOD_PICKLE:
            return "I'm Pickle Rick! ...wait, no.";
        default:
            return RICK_QUOTES[random(0, quoteCount)];
    }
}

// =============================================================================
// ANIMATION
// =============================================================================
void rick_update_animation(rick_avatar_t* rick) {
    // Blink every 3-7 seconds
    if (millis() - rick->lastBlink > (3000 + random(0, 4000))) {
        rick->lastBlink = millis();
        // Trigger blink animation
        rick->isAnimating = true;
    }

    // Random mood shifts
    rick_random_mood(rick);
}

// =============================================================================
// RANK SYSTEM
// =============================================================================
uint32_t rick_xp_for_rank(rick_rank_t rank) {
    if (rank >= RANK_MAX) return RANK_XP_THRESHOLDS[RANK_MAX - 1];
    return RANK_XP_THRESHOLDS[rank];
}

rick_rank_t rick_calculate_rank(uint32_t xp) {
    for (int i = RANK_MAX - 1; i >= 0; i--) {
        if (xp >= RANK_XP_THRESHOLDS[i]) {
            return (rick_rank_t)i;
        }
    }
    return RANK_MORTY;
}

const char* rick_rank_name(rick_rank_t rank) {
    if (rank >= RANK_MAX) return "Unknown";
    return RANK_NAMES[rank];
}

const char* rick_rank_icon(rick_rank_t rank) {
    static const char* icons[] = {
        "🥴", // Morty
        "📱", // Summer
        "🐴", // Beth
        "😰", // Jerry
        "👋", // Mr. Meeseeks
        "😈", // Scary Terry
        "🦅", // Birdperson
        "🎉", // Squanchy
        "💜", // Unity
        "🔫", // Krombopulos
        "🎭", // Evil Morty
        "🥒", // Pickle Rick
        "☣️", // Toxic Rick
        "🎸", // Tiny Rick
        "🍪", // Doofus Rick
        "👑", // Council Rick
        "⭐", // Rick Prime
        "🌀", // C-137
        "🚀", // Portal Master
        "🌌", // Dimension Hopper
        "🔱"  // Multiverse God
    };
    if (rank >= RANK_MAX) return "❓";
    return icons[rank];
}
