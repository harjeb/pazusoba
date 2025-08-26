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
#include <map>
#include <cmath>
#include <string>
#include <iostream>
#include "board.h"
#include "pad.h"
#include "configuration.h"

// The base of all profiles, it only requires a getScore function
class Profile
{
public:
    int row = Configuration::shared().getRow();
    int column = Configuration::shared().getColumn();
    int minErase = Configuration::shared().getMinErase();

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
    // Return a shared instance of profile manager
    static ProfileManager &shared()
    {
        static ProfileManager p;
        return p;
    }

    // Loop through all profiles and add scores based on every profile
    // You need a list of combos, the current board and moveCount
    int getScore(const ComboList &list, const Board &board, int moveCount)
    {
        int score = 0;

        for (auto &p : profiles)
        {
            score += p->getScore(list, board, moveCount);

        }
        //std::cout << score;
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
    void updateProfile(const std::vector<Profile *> &p)
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
        

        // Check if there are orbs next to each other
        int orbAround = 0;
        int orbNext2 = 0;

        // Collect all orbs with current location
        // std::map<Orb, std::vector<OrbLocation>> distanceInfo;

        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < column; j++)
            {
                auto curr = board[INDEX_OF(i, j)];
                if (curr == pad::empty)
                    continue;

                // TODO: improve this??
                // save this location
                // distanceInfo[curr].push_back(LOCATION(i, j));

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
                        if (x >= 0 && x < row && y >= 0 && y < column)
                        {
                            // Check orbs are the same
                            auto orb = board[INDEX_OF(i, j)];
                            if (curr == orb)
                            {
                                orbAround++;
                                if ((a == 0 && ((b == 1) || (b == -1))) ||
                                    (b == 0 && ((a == 1) || (a == -1))))
                                {
                                    // This means that it is a line
                                    orbNext2 += 1;
                                    orbAround -= 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        // For every orbs, we need to get the distance of it from other orbs
        // for (auto curr = distanceInfo.begin(); curr != distanceInfo.end(); curr++)
        // {
        //     // track the total distance
        //     int distance = 0;
        //     auto orbs = curr->second;
        //     int size = orbs.size();
        //     for (int i = 0; i < size; i++)
        //     {
        //         auto loc = orbs[i];
        //         for (int j = i; j < size; j++)
        //         {
        //             auto other = orbs[j];
        //             distance += abs(loc.first - other.first) + abs(loc.second - other.second);
        //             // distance += (int)sqrt(pow(loc.first - other.first, 2) + pow(loc.second - other.second, 2));
        //         }
        //     }

        //     // Less points if far away
        //     score -= pad::TIER_THREE_SCORE * distance;
        // }

        if (targetCombo == 0)
        {
            // Aim for zero combo and make sure orbs are not close to each other
            score -= pad::TIER_ONE_SCORE * orbAround;
            score -= pad::TIER_TWO_SCORE * orbNext2;
            score -= pad::TIER_FOUR_SCORE * moveCount;
            score -= pad::TIER_FIVE_SCORE * combo;
        }
        else
        {
            if (targetCombo > 0)
            {
                int distance = combo - targetCombo;
                if (distance > 0)
                {
                    // Sometimes, it is ok to do more combo temporarily
                    combo *= -1;
                }
            }
            else
            {
                // Encourage cascading
                score += pad::TIER_FOUR_SCORE * moveCount;
            }

            // Always aim for max combo by default
            score += pad::TIER_ONE_SCORE * orbAround;
            score += pad::TIER_TWO_SCORE * orbNext2;
            score += pad::TIER_FIVE_SCORE * combo;
            //std::cout << combo << "###############\n";
        }
        //std::cout << combo << "~~~~~~~~~\n";
        return score;
    }
};

// More points for more colour erased
class ColourProfile : public Profile
{
    // By default, all main colours
    std::vector<Orb> orbs;

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
            auto orb = c[0].orb;
            if (orbs.size() == 0)
            {
                colours.insert(orb);
            }
            else
            {
                for (const auto &o : orbs)
                {
                    if (orb == o)
                    {
                        // Only insert if matches
                        colours.insert(orb);
                        break;
                    }
                }
            }
        }

        // Check if colours matches
        score += colours.size() * pad::TIER_SEVEN_SCORE;
        //std::cout << score << "!!!!!!!!!!!!\n";
        return score;
    }
};

// More points if a combo has a certain shape
class ShapeProfile : public Profile
{
    std::vector<Orb> orbs;

public:
    ShapeProfile() {}
    ShapeProfile(std::vector<Orb> o) : orbs(o) {}

    // Check if orb list contains this orb
    virtual bool isTheOrb(Orb orb) const
    {
        if (orbs.size() == 0)
            return true;

        for (const auto &o : orbs)
        {
            if (orb == o)
            {
                return true;
            }
        }

        return false;
    }
};

class TwoWayProfile : public ShapeProfile
{
public:
    TwoWayProfile() : ShapeProfile() {}
    TwoWayProfile(std::vector<Orb> orbs) : ShapeProfile(orbs) {}

    std::string getProfileName() const override
    {
        return "2U";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        for (const auto &c : list)
        {
            // recovery is not two way
            if (c[0].orb == pad::recovery)
                continue;

            // 2U needs 4 orbs connected
            if (c.size() == 4 && isTheOrb(c[0].orb))
            {
                score += pad::TIER_SIX_SCORE;

            }
        }
        //std::cout << score << "~~~~~~~~~\n";
        return score;
    }
};

class LProfile : public ShapeProfile
{
public:
    LProfile() : ShapeProfile() {}
    LProfile(std::vector<Orb> orbs) : ShapeProfile(orbs) {}

    std::string getProfileName() const override
    {
        return "L";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        for (const auto &c : list)
        {
            // L shape must erase 5 orbs and right colour
            if (c.size() == 5 && isTheOrb(c[0].orb))
            {
                std::map<int, int> vertical;
                std::map<int, int> horizontal;
                int bigFirst = -1;
                int bigSecond = -1;

                // Collect info
                for (const auto &loc : c)
                {
                    int x = loc.first;
                    int y = loc.second;
                    vertical[x]++;
                    horizontal[y]++;

                    // Track the largest number
                    if (vertical[x] >= 3)
                        bigFirst = x;
                    if (horizontal[y] >= 3)
                        bigSecond = y;
                }

                // This is the center point
                if (bigFirst > -1 && bigSecond > -1)
                {
                    int counter = 0;
                    // Check if bigFirst -2 or +2 exists
                    if (vertical[bigFirst - 2] > 0 || vertical[bigFirst + 2] > 0)
                        counter++;
                    // Same for bigSecond
                    if (horizontal[bigSecond - 2] > 0 || horizontal[bigSecond + 2] > 0)
                        counter++;

                    if (counter == 2)
                        score += pad::TIER_EIGHT_SCORE;
                }
            }
        }
        return score;
    }
};

class PlusProfile : public ShapeProfile
{
public:
    PlusProfile() : ShapeProfile() {}
    PlusProfile(std::vector<Orb> orbs) : ShapeProfile(orbs) {}

    std::string getProfileName() const override
    {
        return "+";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        for (const auto &c : list)
        {
            // + shape must erase 5 orbs
            if (c.size() == 5 && isTheOrb(c[0].orb))
            {
                std::map<int, int> vertical;
                std::map<int, int> horizontal;
                int bigFirst = -1;
                int bigSecond = -1;

                // Collect info
                for (const auto &loc : c)
                {
                    int x = loc.first;
                    int y = loc.second;
                    vertical[x]++;
                    horizontal[y]++;

                    // Track the largest number
                    if (vertical[x] >= 3)
                        bigFirst = x;
                    if (horizontal[y] >= 3)
                        bigSecond = y;
                }

                // This is the center point
                if (bigFirst > -1 && bigSecond > -1)
                {
                    int counter = 0;
                    // Check up down left right there is an orb around center orb
                    if (vertical[bigFirst - 1] > 0 && vertical[bigFirst + 1] > 0)
                        counter++;
                    if (horizontal[bigSecond - 1] > 0 && horizontal[bigSecond + 1] > 0)
                        counter++;

                    if (counter == 2)
                        score += pad::TIER_TEN_SCORE * 2;
                }
            }
        }
        return score;
    }
};



class NineProfile : public ShapeProfile
{
public:
    NineProfile() : ShapeProfile() {}
    NineProfile(std::vector<Orb> orbs) : ShapeProfile(orbs) {}

    std::string getProfileName() const override
    {
        return "9";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        for (const auto &c : list)
        {
            // must erase 9 orbs
            if (c.size() >= 9 && isTheOrb(c[0].orb))
            {
                std::map<int, int> vertical;
                std::map<int, int> horizontal;
                int bigFirst = -1;
                int bigSecond = -1;

                // Collect info
                for (const auto &loc : c)
                {
                    int x = loc.first;
                    int y = loc.second;
                    vertical[x]++;
                    horizontal[y]++;


                    // Track the largest number
                    if (vertical[x] >= 3)
                        bigFirst = x;
                    if (horizontal[y] >= 3)
                        bigSecond = y;
                }

                // This is the center point
                if (bigFirst > -1 && bigSecond > -1)
                {

                    int count = 0;
                    for (auto curr = vertical.begin(); curr != vertical.end(); curr++)
                    {
                        auto value = curr->second;
                        if (value == 3)
                            count++;
                    }
                    for (auto curr = horizontal.begin(); curr != horizontal.end(); curr++)
                    {
                        auto value = curr->second;
                        if (value == 3)
                            count++;
                    }

                    if (count >= 6)
                    {
                        score += count * pad::TIER_NINE_SCORE;
                    }
                }
            }
        }
        return score;
    }
};









// Void damage penetration
class VoidPenProfile : public ShapeProfile
{
public:
    VoidPenProfile() : ShapeProfile() {}
    VoidPenProfile(std::vector<Orb> orbs) : ShapeProfile(orbs) {}

    std::string getProfileName() const override
    {
        return "void damage penetration";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        // Try this board
        // DHLHHDHDDDHLDDHDLLLHHLHLLLLDHLHLDLHLLLHLHH
        //std::cout << "--------------------------\n\n";

        int score = 0;
        int a = 0;
        int b = 0;
        int cc = 0;
        int d = 0;
        int e = 0;

        for (const auto &c : list)
        {
            //std::cout <<  "0~\n";    
            auto orb = c[0].orb;
            // More points if it is a 3x3 shape
            if (isTheOrb(orb)  && c.size() == 9)
            {
                //std::cout  <<  "~~~\n";
                int size = c.size();
                int distance = size - 9;

                if (size == minErase)
                {
                    score -= pad::TIER_ONE_SCORE;
                    a = score;
                }

                if (size == 9)
                {
                    // std::cout << size << "~~~~~~~~~\n";
                    // Must connect more than min erase condition
                    score += size * pad::TIER_ONE_SCORE;
                    b = score;

                    // Do the same like + and L
                    std::map<int, int> vertical;
                    std::map<int, int> horizontal;

                    // Collect info
                    for (const auto &loc : c)
                    {
                        int x = loc.first;
                        int y = loc.second;
                        vertical[x]++;
                        horizontal[y]++;
                    }

                    int v = vertical.size();
                    int h = horizontal.size();
                    if (v < 4 && h < 4)
                    {
                        score += (v + h) * pad::TIER_ONE_SCORE;
                        cc = score;
                    }

                    // All x and y are 3 because it is 3x3
                    if (v == 3 && h == 3)
                    {
                        std::cout  <<  "~~~\n";
                        int count = 0;
                        for (auto curr = vertical.begin(); curr != vertical.end(); curr++)
                        {
                            auto value = curr->second;
                            if (value == 3)
                                count++;
                        }
                        for (auto curr = horizontal.begin(); curr != horizontal.end(); curr++)
                        {
                            auto value = curr->second;
                            if (value == 3)
                                count++;
                        }

                        if (count < 6)
                            score += count * pad::TIER_EIGHT_SCORE;
                        if (count == 6)
                            {
                            score += count * pad::TIER_NINE_SCORE;
                            d = score;
                            }

                    }
                }
                else
                {
                    score -= distance * pad::TIER_ONE_SCORE;
                    e = score;
                }
            }
        }
        if (score > 1000 )
        {
            std::cout << a <<  " ~\n";
            std::cout << b <<  " ~\n";
            std::cout << cc <<  " ~\n";
            std::cout << d <<  " ~\n";
            std::cout << e <<  " ~\n";
            std::cout << score <<  "~~!!!!!~\n";
        }

        return score;
    }
};

// Connect 10 - 12 orbs together and you get +1 combo and a green soybean
class SoybeanProfile : public ShapeProfile
{
public:
    SoybeanProfile() : ShapeProfile() {}
    SoybeanProfile(std::vector<Orb> orbs) : ShapeProfile(orbs) {}

    std::string getProfileName() const override
    {
        return "soybean";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        for (const auto &c : list)
        {
            int size = c.size();
            if (size >= 10 && size <= 12 && isTheOrb(c[0].orb))
            {
                score += pad::TIER_NINE_SCORE;
            }
        }
        return score;
    }
};

class OneRowProfile : public ShapeProfile
{
public:
    OneRowProfile() : ShapeProfile() {}
    OneRowProfile(std::vector<Orb> orbs) : ShapeProfile(orbs) {}

    std::string getProfileName() const override
    {
        return "row";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        for (const auto &c : list)
        {
            int size = c.size();
            // It has a row, xxxxxx or more
            if (size == row && isTheOrb(c[0].orb))
            {
                // Record all encountered x
                std::map<int, int> xs;
                for (const auto &loc : c)
                {
                    // Track x because it should have the number of row
                    xs[loc.first]++;
                }

                // Sometimes, it is a long L so just check which x has more than row times
                for (auto curr = xs.begin(); curr != xs.end(); curr++)
                {
                    if (curr->second == row)
                    {
                        score += pad::TIER_NINE_SCORE;
                        break;
                    }
                }
            }
        }
        return score;
    }
};

class OneColumnProfile : public ShapeProfile
{
public:
    OneColumnProfile() : ShapeProfile() {}
    OneColumnProfile(std::vector<Orb> orbs) : ShapeProfile(orbs) {}

    std::string getProfileName() const override
    {
        return "column";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        for (const auto &c : list)
        {
            int size = c.size();
            // It is a column
            if (size == column && isTheOrb(c[0].orb))
            {
                // Record all encountered x
                std::map<int, int> xs;
                for (const auto &loc : c)
                {
                    // Track x because it should have the number of row
                    xs[loc.second]++;
                }

                // Sometimes, it is a long L so just check which x has more than row times
                for (auto curr = xs.begin(); curr != xs.end(); curr++)
                {
                    if (curr->second == column)
                    {
                        score += pad::TIER_EIGHT_PLUS_SCORE;
                        break;
                    }
                }
            }
        }
        return score;
    }
};

// More points if there are less orbs left
class OrbProfile : public Profile
{
    // Aim for nothing less
    int targetNumber = 0;

public:
    OrbProfile() {}
    OrbProfile(int count) : targetNumber(count) {}

    std::string getProfileName() const override
    {
        return "orb remains";
    }

    int getScore(const ComboList &list, const Board &board, int moveCount) const override
    {
        int score = 0;
        if (list.size() == 0)
            return score;

        int orbCount = board.size();

        // Track how many orbs are erased and left
        int orbErased = 0;
        int orbLeft = 0;

        // Encourage to connect more orbs in a combo
        for (const auto &c : list)
        {
            int size = c.size();
            score += (size - minErase) * pad::TIER_FIVE_SCORE;
            orbErased += size;
        }

        orbLeft = orbCount - orbErased;
        // More points for erasing more orbs
        score -= pad::TIER_SIX_SCORE * orbLeft;
        if (orbLeft <= targetNumber)
        {
            score += pad::TIER_NINE_SCORE;
        }

        return score;
    }
};

#endif
