/**
 * pad.h
 * by Yiheng Quan
 */

#ifndef PAD_H
#define PAD_H

#include <string>

namespace pad
{
    /**
     * All orbs in the game.
     * 
     * seal is a tape over a row and orbs cannot move,
     * disabled means it will not get erased even if connected.
     */
    enum orbs
    {
        empty,
        fire,
        water,
        wood,
        light,
        dark,
        recovery,
        jammer,
        bomb,
        poison,
        poison_plus,
        seal,
        disabled,
        unknown
    };
    const int ORB_COUNT = 14;
    // For displaying the orb name
    const std::string ORB_NAMES[ORB_COUNT] = {"", "Fire", "Water", "Wood", "Light", "Dark", "Heal", "Jammer", "Bomb", "Poison", "Poison+", "Tape", "-X-", "???"};
    // Some emulation websites use these names for orbs (not all orbs are supported)
    const std::string ORB_SIMULATION_NAMES[ORB_COUNT] = {"", "R", "B", "G", "L", "D", "H", "J", "", "P", "", "", "", ""};

    // TODO: consider weight to adjust the heuristic

    // All 8 possible directions to move to
    enum direction
    {
        upLeft,
        up,
        upRight,
        left,
        right,
        downLeft,
        down,
        downRight
    };
    // Direction names
    const std::string DIRECTION_NAMES[8] = {"UL", "U", "UR", "L", "R", "DL", "D", "DR"};

    // This is for all profiles and based on how important it is
    enum score
    {
        TIER_ONE_SCORE = 1,
        TIER_TWO_SCORE = 5,
        TIER_THREE_SCORE = 10,
        TIER_FOUR_SCORE = 20,
        TIER_FIVE_SCORE = 50,
        TIER_SIX_SCORE = 100,
        TIER_SEVEN_SCORE = 500,
        TIER_EIGHT_SCORE = 1000,
        TIER_NINE_SCORE = 5000,
        TIER_TEN_SCORE = 10000,
    };
} // namespace pad

#endif
