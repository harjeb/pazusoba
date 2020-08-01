/**
 * profile.h
 * P&D has many teams and profiles are for different teams and play style
 * 
 * by Yiheng Quan
 */

#ifndef PROFILE_H
#define PROFILE_H

#include <vector>
#include <set>
#include <string>
#include "board.h"
#include "pad.h"

// The base of all profiles, it only requires a getScore function
class Profile
{
public:
    virtual ~Profile() {}
    virtual std::string getProfileName() const = 0;
    virtual int getScore(const ComboList &list, const Board &board, int moveCount) const = 0;
};

// This is a singleton class to update profiles at run time
class ProfileManager
{
    std::vector<Profile *> profiles;
    ProfileManager() {}

public:
    ~ProfileManager()
    {
        clear();
    }

    // Return a shared instance of profile manager
    static ProfileManager &shared()
    {
        static ProfileManager p;
        return p;
    }

    // Loop through all profiles and add scores based on every profile
    // You need a list of combos, the current board and moveCount
    int getScore(ComboList &list, Board &board, int moveCount)
    {
        int score = 0;

        for (auto &p : profiles)
        {
            score += p->getScore(list, board, moveCount);
        }

        return score;
    }

    void clear()
    {
        // Delete all profiles
        for (const auto &p : profiles)
        {
            delete p;
        }
        profiles.clear();
    }

    // Add a profile to current list
    void addProfile(Profile *p)
    {
        profiles.push_back(p);
    }

    // Clear all profiles and set it to a new one
    void updateProfile(std::vector<Profile *> &p)
    {
        clear();
        for (const auto &p : profiles)
        {
            delete p;
        }
        profiles.clear();
        profiles = p;
    }
};

// More points for more combos
class ComboProfile : public Profile
{
    // If it is-1, always aim for more combo
    int targetCombo = -1;

public:
    // Just a default profile
    ComboProfile() {}
    // With a target combo
    ComboProfile(int combo) : targetCombo(combo) {}

    std::string getProfileName() const override
    {
        return "combo";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        int combo = list.size();
        // Get column and row based on board size
        int column = board.size();
        int row = board[0].size();

        // Check if there are orbs next to each other
        int orbAround = 0;
        int orbNearby = 0;
        for (int i = 0; i < column; i++)
        {
            for (int j = 0; j < row; j++)
            {
                auto curr = board[i][j];
                if (curr == pad::empty)
                    continue;

                // Check if there are same orbs around
                for (int a = -1; a <= 1; a++)
                {
                    for (int b = -1; b <= 1; b++)
                    {
                        // This is the current orb
                        if (a == 0 && b == 0)
                            continue;

                        int x = i + a, y = j + b;
                        // check x & y are valid
                        if (x >= 0 && x < column && y >= 0 && y < row)
                        {
                            // Check orbs are the same
                            auto orb = board[x][y];
                            if (curr == orb)
                            {
                                orbAround++;
                                if ((a == 0 && ((b == 1) || (b == -1))) ||
                                    (b == 0 && ((a == 1) || (a == -1))))
                                {
                                    // This means that it is a line
                                    orbNearby += 1;
                                    orbAround -= 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (targetCombo < 0)
        {
            // Always aim for max combo
            score += pad::ORB_AROUND_SCORE * orbAround;
            score += pad::ORB_NEARBY_SCORE * orbNearby;
            score += pad::ONE_COMBO_SCORE * combo;
            score += pad::CASCADE_SCORE * moveCount;
        }
        else if (targetCombo == 0)
        {
            // Aim for zero combo and make sure orbs are not close to each other
            score -= pad::ORB_AROUND_SCORE * orbAround;
            score -= pad::ORB_NEARBY_SCORE * orbNearby;
            score -= pad::ONE_COMBO_SCORE * combo;
            score -= pad::CASCADE_SCORE * moveCount;
        }
        else
        {
            // Make sure to get the absolute path for combo
            // If you want 5 combo, 6 combo is liek 4 combo
            combo = targetCombo - abs(targetCombo - combo);
            score += pad::ORB_AROUND_SCORE * orbAround;
            score += pad::ORB_NEARBY_SCORE * orbNearby;
            score += pad::ONE_COMBO_SCORE * combo;
            score += pad::CASCADE_SCORE * moveCount;
        }

        return score;
    }
};

// More points for more colour erased
class ColourProfile : public Profile
{
    // By default, all main colours
    std::vector<Orb> orbs{pad::fire, pad::water, pad::wood, pad::light, pad::dark, pad::recovery};

public:
    ColourProfile() {}
    ColourProfile(std::vector<Orb> o) : orbs(o) {}

    std::string getProfileName() const override
    {
        return "colour";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        std::set<Orb> colours;
        for (const auto &c : list)
        {
            int x = c[0].first, y = c[0].second;
            colours.insert(board[x][y]);
        }

        // Check if colours matches
        score += colours.size() * 500;
        return score;
    }
};

// More points if a combo has a certain shape
class ShapeProfile : public Profile
{
    // This determine which orb we are targetting
    virtual std::vector<Orb> targetOrbs();
};

class TwoWayProfile : public ShapeProfile
{
public:
    std::string getProfileName() const override
    {
        return "2U";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        for (const auto &c : list)
        {
            // 2U needs 4 orbs connected
            if (c.size() == 4)
                score += 500;
        }
        return score;
    }
};

// More points if there are less orbs left
class OrbProfile : public Profile
{
    int targetNumber = -1;

public:
    OrbProfile() {}
    OrbProfile(int count) : targetNumber(count) {}

    std::string getProfileName() const override
    {
        return "orb remains";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int boardSize = board.size() * board[0].size();
        // -1 means no target so why use this?
        if (targetNumber < 0 || targetNumber > boardSize)
            return 0;

        int score = 0;
        int orbRemain = 0;
        // More points for connecting more
        for (const auto &c : list)
        {
            score += c.size() * 100;
        }
        score -= (orbRemain - targetNumber) * 500;
        return score;
    }
};

#endif
