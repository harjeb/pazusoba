/**
 * route.h
 * This saves routes from states and prints a readable path
 * 
 * by Yiheng Quan
 */

#ifndef ROUTE_H
#define ROUTE_H

#include <vector>
#include <iostream>
#include "board.h"
#include "state.h"
#include "pad.h"

typedef std::vector<pad::direction> Directions;

class Route
{
    PState *state;
    PBoard finalBoard;
    PBoard erasedBoard;
    // A list of direction
    Directions directions;
    OrbLocation start;
    int score;
    int step;
    int combo;

    // Convert the end state to Directions
    void convertFromState(const PState *s);

    // Go all the way back to the start and push to directions
    void stateBack(const PState *curr, const PState *parent);

    // Convert orbLocation to a direction
    pad::direction getDirection(const OrbLocation &curr, const OrbLocation &prev);

public:
    Route(PState *state);

    void printRoute();
    void printErasedBoard();
    void printFinalBoard();  // Display final board state after moves
    std::string getFinalBoardString() const;  // Get final board as string
    std::string getFinalBoardStringMultiLine() const;  // Get final board as multi-line string
    inline int getStep() { return step; }
    inline int getCombo() { return combo; }
    inline int getScore() { return score; }
    inline void saveToDisk() { state->saveToDisk(); }
};

#endif
