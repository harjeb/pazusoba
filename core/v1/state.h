/**
 * state.h
 * by Yiheng Quan
 */

#ifndef STATE_H
#define STATE_H

#include <fstream>
#include <mutex>
#include "board.h"

class PState
{
    // A list of state
    typedef std::vector<PState *> PStateList;

    // This is max step we want to go (termination point)
    int maxStep;
    // ALl possible moves from this state
    PStateList children;
    std::mutex mtx;

    // This is a recursive function to print from the beginning
    void printStateFromRoot(const PState *parent);
    // This is a recursive function to save state from the beginning
    void saveStateFromRoot(const PState *parent, std::ofstream* file);

public:
    // The board
    PBoard board;
    PBoard erasedBoard;
    // This score shows how good the current board is
    int score = 0;
    // This is the number of steps (depth)
    int step;
    // This is the previous orb and it moved to current orb
    // Current must not move back to parent location
    OrbLocation previous;
    // This is current orb which is active (the player is holding it)
    OrbLocation current;
    // Save the parent's address to track back
    PState *parent = nullptr;
    
    // 9-grid constraint variables
    static bool useNineGridConstraint;
    static OrbLocation nineGridCenter;
    
    // Diagonal movement control
    static bool enableDiagonalMovement;
    
    // Check if a location is within the 3x3 grid constraint
    bool isWithinNineGrid(const OrbLocation &location) const;

    // This is only for testing
    PState(int s) : score(s) {}
    PState(const PBoard &board, const OrbLocation &from, const OrbLocation &to, int step, int maxStep);
    ~PState();

    // Override operators
    bool operator<(const PState &a) const;
    bool operator>(const PState &a) const;
    bool operator==(const PState &a) const;

    // This returns a list of all possible children states
    PStateList getChildren();

    // Print out current state, its board, score and more
    void printState();

    // Save the entire state from begining to the end to the disk
    void saveToDisk();
    
    // Static methods for 9-grid constraint
    static void setNineGridConstraint(bool enable, const OrbLocation &center = OrbLocation(0,0));
    static bool isNineGridEnabled() { return useNineGridConstraint; }
    
    // Static methods for diagonal movement control
    static void setDiagonalMovementEnabled(bool enable);
    static bool isDiagonalMovementEnabled() { return enableDiagonalMovement; }
};

#endif
