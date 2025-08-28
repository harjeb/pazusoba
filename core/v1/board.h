/**
 * board.h
 * by Yiheng Quan
 */

#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <array>
#include <set>
#include <functional>
#include "pad.h"
#include "configuration.h"

/// Convert location to index
#define INDEX_OF(x, y) (x * column + y)
#define MAX_BOARD_SIZE 42

/// Another name for orb enum from pad.h
typedef pad::orbs Orb;
/// Board is an array of Orb, for now max 7x6 so 42
typedef std::array<Orb, MAX_BOARD_SIZE> Board;

/// Convert index to include first (x) and second (y)
struct OrbLocation
{
    int index = -1;
    int first;
    int second;
    int column = Configuration::shared().getColumn();

    bool operator==(const OrbLocation &loc) const
    {
        return index == loc.index;
    }

    OrbLocation() {}
    OrbLocation(int index) : index(index)
    {
        first = index / column;
        second = index % column;
    }
    // column can be ignored sometimes
    OrbLocation(int first, int second) : first(first), second(second)
    {
        // first and second are all indices so need to add one here
        index = first * column + second;
    }
};

/// Struct for storing combo info
struct ComboInfo
{
    int first;
    int second;
    Orb orb;
    ComboInfo(int f, int s, const Orb &o) : first(f), second(s), orb(o) {}
};
typedef std::vector<ComboInfo> Combo;
typedef std::vector<Combo> ComboList;

class PBoard
{
    int row;
    int column;
    int minErase;

    /// This saves all orbs in an array, support all orb types
    Board board;
    /// Used for flood fill
    std::array<int, MAX_BOARD_SIZE> temp;

    /// Move orbs down if there is an empty orb below, return whether board has been changed
    bool moveOrbsDown();

    /// Search for a combo and erase orbs
    void floodfill(Combo *list, const OrbLocation &loc, const Orb &orb, bool initial);

    // Erase all combos, move orbs down and track the move count
    ComboList eraseComboAndMoveOrbs(int *moveCount);

    inline bool hasSameOrb(const Orb &orb, const OrbLocation &loc)
    {
        if (validLocation(loc))
        {
            return board[loc.index] == orb;
        }

        return false;
    }

    /**
     * Calculate max combo from a list of orbs.
     * NOTE that this is not the true MAX COMBO possible,
     * but it represents the max combo an averge player can do.
     */
    int getMaxCombo(int *counter);

    /// A quick and easy way of getting max combo
    inline int getBoardMaxCombo()
    {
        return row * column / minErase;
    }

    /// Check if the file is empty or doesn't exists
    inline bool isEmptyFile()
    {
        return column == 0 && row == 0;
    }

    inline int getIndex(int x, int y)
    {
        return y * column + x;
    }

    /// Loop through the vector and count the number of each orbs
    inline int *collectOrbCount()
    {
        int *counter = new int[pad::ORB_COUNT]{0};
        traverse([&](int i, int j, Orb orb) {
            counter[orb]++;
        });
        return counter;
    }

public:
    PBoard() {}
    PBoard(const Board &board);

    /// Rate current board with profiles
    int rateBoard();

    /// Get combo count from current board
    int getComboCount();

    /// Print out a board nicely formatted
    void printBoard();

    /// Print out all orbs in a line and it can be used for simulation
    void printBoardForSimulation();

    /// Print out some info about the board we have
    void printBoardInfo();
    
    /// Get board as string (RHGHDR format)
    std::string getBoardString() const;
    
    /// Simulate combo elimination and orb falling (public version for prediction)
    ComboList simulateComboElimination(int *moveCount);
    
    /// Get current board state as Board array
    Board getBoardArray() const;
    
    /// Get board as multi-line string (RHGHDR format with line breaks)
    std::string getBoardStringMultiLine() const;

    /// Traverse through the board
    inline void traverse(std::function<void(int, int, Orb)> func)
    {
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < column; j++)
            {
                auto orb = board[INDEX_OF(i, j)];
                func(i, j, orb);
            }
        }
    }
    
    /// Traverse through the board (const version)
    inline void traverse(std::function<void(int, int, Orb)> func) const
    {
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < column; j++)
            {
                auto orb = board[INDEX_OF(i, j)];
                func(i, j, orb);
            }
        }
    }

    inline bool hasSameBoard(const PBoard *b) const
    {
        return board == b->board;
    }

    /// Swap the value of two orbs
    inline void swapLocation(const OrbLocation &one, const OrbLocation &two)
    {
        if (!validLocation(one) || !validLocation(two))
            return;
            
        // Seal orbs (S珠) cannot be moved
        if (board[one.index] == pad::seal || board[two.index] == pad::seal)
            return;

        auto temp = board[one.index];
        board[one.index] = board[two.index];
        board[two.index] = temp;
    }

    inline bool validLocation(const OrbLocation &loc)
    {
        // Check if it is in bound, check for orb in the future?
        int x = loc.first;
        int y = loc.second;
        if (x >= 0 && x < row && y >= 0 && y < column)
        {
            return board[loc.index] != pad::unknown;
        }

        return false;
    }
};

#endif
