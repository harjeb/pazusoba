/**
 * board.cpp
 * by Yiheng Quan
 */

#include <iostream>
#include <sstream>
#include <array>
#include <set>
#include "board.h"
#include "profile.h"
#include "configuration.h"

// MARK: - Constructors
PBoard::PBoard(const Board &board)
{
    this->board = board;
    this->temp.fill(0);

    auto config = Configuration::shared();
    this->row = config.getRow();
    this->column = config.getColumn();
    this->minErase = config.getMinErase();
}

// MARK: - Board related
ComboList PBoard::eraseComboAndMoveOrbs(int *moveCount)
{
    // Reverse to prevent slow pushing back
    ComboList comboList;
    comboList.reserve(row * column / minErase);
    Combo combo;
    combo.reserve(row * column);

    bool moreCombo;
    do
    {
        // Remember to reset
        moreCombo = false;

        // from bottom left to top right
        for (int i = row - 1; i >= 0; i--)
        {
            for (int j = 0; j < column; j++)
            {
                auto loc = OrbLocation(i, j);
                auto orb = board[loc.index];
                // Ignore empty orbs, seal orbs (cannot form combos), and random orbs (unpredictable)
                if (orb == pad::empty || orb == pad::seal || orb == pad::random)
                    continue;

                // Start finding combos
                floodfill(&combo, loc, orb, true);
                // reset visited orbs
                for (const auto &orb : combo)
                {
                    temp[INDEX_OF(orb.first, orb.second)] = 0;
                }

                if ((int)combo.size() > 0)
                {
                    moreCombo = true;
                    comboList.push_back(combo);
                    combo.clear();
                }
            }
        }

        // Only move orbs down if there is at least one combo
        if (moreCombo)
        {
            // If the board is not moved, there won't be new combos so break;
            moreCombo = moveOrbsDown();
            *moveCount += 1;
        }
    } while (moreCombo);

    return comboList;
}

void PBoard::floodfill(Combo *list, const OrbLocation &loc, const Orb &orb, bool initial)
{
    if (!validLocation(loc))
        return;

    auto currOrb = board[loc.index];
    // Seal orbs and random orbs cannot participate in combos
    if (currOrb == pad::seal || currOrb == pad::random)
        return;
    // only accept if current orb is the target orb
    if (currOrb != orb && temp[loc.index] < 1)
        return;

    // track num of connected orbs, also include the current orb so start from 1
    int count = 1;
    // track all directions
    int dList[4] = {0, 0, 0, 0};
    // the min number of connected orbs to consider it as a combo
    int minConnection = initial ? minErase : (minErase >= 3 ? 3 : minErase);

    int x = loc.first;
    int y = loc.second;

    // all 4 directions
    // 0 -> right
    // 1 -> left
    // 2 -> down
    // 3 -> up
    for (int d = 0; d < 4; d++)
    {
        int loop = row;
        if (d > 1)
            loop = column;

        // start from 1 to look for orbs around
        for (int i = 1; i < loop; i++)
        {
            int cx = x;
            int cy = y;
            if (d == 0)
                cy += i;
            else if (d == 1)
                cy -= i;
            else if (d == 2)
                cx += i;
            else if (d == 3)
                cx -= i;

            auto loc = OrbLocation(cx, cy);
            if (!validLocation(loc))
                break;

            // stop if it doesn't match and it is not visited yet
            // if visited, it means this orb has the same colour
            if (board[loc.index] != orb && temp[loc.index] < 1)
                break;

            dList[d]++;
            count++;
        }
    }

    // more than erase condition
    if (count >= minConnection)
    {
        int start = 0;
        int end = 2;

        int hCount = dList[0] + dList[1] + 1;
        int vCount = dList[2] + dList[3] + 1;
        bool horizontal = hCount >= minConnection;
        bool vertical = vCount >= minConnection;
        // this is either L or +
        if (hCount == 3 && vCount == 3)
        {
            horizontal = true;
            vertical = true;
        }

        if (!horizontal)
            start = 1;
        if (!vertical)
            end = 1;

        // erase and do flood fill
        for (int d = start; d < end; d++)
        {
            int startCount = -(dList[d * 2 + 1]);
            int endCount = dList[d * 2] + 1;

            for (int i = startCount; i < endCount; i++)
            {
                int cx = x;
                int cy = y;
                if (d == 0)
                    cy += i;
                else if (d == 1)
                    cx += i;

                int index = INDEX_OF(cx, cy);
                if (temp[index] == 0)
                {
                    board[index] = pad::empty;
                    list->emplace_back(cx, cy, orb);
                    temp[index] = 1;
                }
                else
                {
                    // shouldn't visit this orb again
                    temp[index] = 2;
                }
            }
        }

        for (int d = start; d < end; d++)
        {
            int startCount = -(dList[d * 2 + 1]);
            int endCount = dList[d * 2] + 1;
            for (int i = startCount; i < endCount; i++)
            {
                int cx = x;
                int cy = y;
                if (d == 0)
                    cy += i;
                else if (d == 1)
                    cx += i;

                // prevent going to the same orb again
                if (temp[INDEX_OF(cx, cy)] < 2)
                {
                    if (d == 0)
                    {
                        // horizontal so fill vertically
                        floodfill(list, OrbLocation(cx + 1, cy), orb, false);
                        floodfill(list, OrbLocation(cx - 1, cy), orb, false);
                    }
                    else
                    {
                        floodfill(list, OrbLocation(cx, cy + 1), orb, false);
                        floodfill(list, OrbLocation(cx, cy - 1), orb, false);
                    }
                }
            }
        }
    }
}

int PBoard::rateBoard()
{
    int moveCount = 0;
    auto list = eraseComboAndMoveOrbs(&moveCount);

    int score = ProfileManager::shared().getScore(list, board, moveCount);
    return score;
}

int PBoard::getComboCount()
{
    int moveCount = 0;
    auto list = eraseComboAndMoveOrbs(&moveCount);
    return list.size();
}

bool PBoard::moveOrbsDown()
{
    bool changed = false;
    for (int j = 0; j < column; j++)
    {
        std::vector<std::pair<pad::orbs, int>> orbsWithPos;
        orbsWithPos.reserve(row);
        
        // First pass: collect all non-empty, non-seal orbs and their positions
        for (int i = row - 1; i >= 0; i--)
        {
            auto orb = board[INDEX_OF(i, j)];
            if (orb != pad::empty && orb != pad::seal)
            {
                orbsWithPos.push_back({orb, i});
            }
        }
        
        // Second pass: place orbs from bottom up, avoiding seal positions
        std::vector<pad::orbs> newColumn(row, pad::empty);
        
        // Keep seal orbs in their original positions
        for (int i = 0; i < row; i++)
        {
            auto orb = board[INDEX_OF(i, j)];
            if (orb == pad::seal)
                newColumn[i] = pad::seal;
        }
        
        // Place movable orbs from bottom up, skipping seal positions
        int orbIndex = 0;
        for (int i = row - 1; i >= 0 && orbIndex < (int)orbsWithPos.size(); i--)
        {
            if (newColumn[i] != pad::seal) // Skip seal positions
            {
                newColumn[i] = orbsWithPos[orbIndex].first;
                orbIndex++;
            }
        }
        
        // Check if column changed and update board
        for (int i = 0; i < row; i++)
        {
            int index = INDEX_OF(i, j);
            if (board[index] != newColumn[i])
            {
                board[index] = newColumn[i];
                changed = true;
            }
        }
    }
    return changed;
}

void PBoard::printBoard()
{
    if (isEmptyFile())
    {
        std::cout << "- empty -\n";
        return;
    }

    // Print everything out nicely
    std::cout << std::endl;
    std::cout << row << " x " << column << std::endl;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            auto orb = board[INDEX_OF(i, j)];
            std::cout << pad::ORB_NAMES[orb] << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PBoard::printBoardForSimulation()
{
    std::stringstream ss;
    for (int i = 0; i < row * column; i++)
    {
        auto orb = board[i];
        auto m= i+1;
        if (m%column==0){
            std::cout << pad::ORB_SIMULATION_NAMES[orb] << "\n";
        }
        else{
            std::cout << pad::ORB_SIMULATION_NAMES[orb];
        }
        ss << (int)(orb - 1);
    }
    std::cout << std::endl;
    std::cout << ss.str() << std::endl;
}

void PBoard::printBoardInfo()
{
    if (isEmptyFile())
    {
        std::cout << "no info\n";
        return;
    }

    // Collect orb info
    int *counter = collectOrbCount();

    // Print out some board info
    for (int i = 1; i < pad::ORB_COUNT; i++)
    {
        int count = counter[i];
        if (count == 0)
            continue;
        // It is just like fire x 5 wood x 6
        std::cout << count << " x " << pad::ORB_NAMES[i];
        if (i != pad::ORB_COUNT - 1)
            std::cout << " | ";
    }
    std::cout << std::endl;

    std::cout << "Board max combo: " << getBoardMaxCombo() << std::endl;
    std::cout << "Current max combo: " << getMaxCombo(counter) << std::endl;

    delete[] counter;
}

std::string PBoard::getBoardString() const
{
    std::string result;
    for (int i = 0; i < row * column; i++)
    {
        auto orb = board[i];
        result += pad::ORB_SIMULATION_NAMES[orb];
    }
    return result;
}

std::string PBoard::getBoardStringMultiLine() const
{
    std::string result;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < column; j++)
        {
            auto orb = board[INDEX_OF(i, j)];
            result += pad::ORB_SIMULATION_NAMES[orb];
        }
        if (i < row - 1)  // Don't add newline after last row
            result += "\n";
    }
    return result;
}

// MARK: - Utils
int PBoard::getMaxCombo(int *counter)
{
    if (isEmptyFile())
        return 0;

    int comboCounter = 0;

    int moreComboCount;
    do
    {
        // Reset this flag everytime
        moreComboCount = 0;
        // This tracks how many orbs are left
        int orbLeft = 0;
        int maxOrbCounter = 0;

        for (int i = 1; i < pad::ORB_COUNT; i++)
        {
            // Keep -3 or other minErase (4, 5) until all orbs are less than 2
            int curr = counter[i];
            if (curr >= minErase)
            {
                moreComboCount += 1;
                comboCounter++;
                counter[i] -= minErase;
                if (curr > maxOrbCounter)
                    maxOrbCounter = curr;
            }
            else
            {
                orbLeft += curr;
            }
        }

        // Only one colour can do combo now
        if (moreComboCount == 1)
        {
            // Orbs left are in different colour but they can still seperate other colours
            int maxComboPossible = orbLeft / minErase;
            int maxCombo = maxOrbCounter / minErase;
            // Make sure there are enough orbs
            comboCounter += maxCombo > maxComboPossible ? maxComboPossible : maxCombo;

            // No orbs left means one colour, -1 here because we count one more but doesn't apply to a single colour board
            if (orbLeft > 0)
                comboCounter--;
            break;
        }
    } while (moreComboCount > 0);

    return comboCounter;
}

ComboList PBoard::simulateComboElimination(int *moveCount)
{
    // This is just a public wrapper for the private eraseComboAndMoveOrbs
    return eraseComboAndMoveOrbs(moveCount);
}

Board PBoard::getBoardArray() const
{
    return board;
}
