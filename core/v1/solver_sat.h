/**
 * solver_sat.h
 * by Yiheng Quan
 */

#ifndef PAD_SAT_SOLVER_H
#define PAD_SAT_SOLVER_H

#include "board.h"
#include "configuration.h"
#include "pad.h"
#include "profile.h"
#include "queue.h"
#include "route.h"
#include "state.h"
#include "timer.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>


/// Solve with SAT, https://en.wikipedia.org/wiki/Boolean_satisfiability_problem
class PSolverSAT {
  int row = 0;
  int column = 0;
  int minErase = 3;
  int steps = 25;
  int size = 1000;
  bool debug = true;

  /// Read board from filePath, return the board
  Board readBoard(const std::string &filePath) {
    Board board;
    board.fill(pad::unknown);
    std::string lines;

    int currIndex = 0;
    std::ifstream boardFile(filePath);
    while (getline(boardFile, lines)) {
      // Ignore lines that start with `//`
      if (lines.find("//") == 0)
        continue;

      // Remove trailing spaces by substr, +1 for substr (to include the char
      // before space)
      int index = lines.find_last_not_of(" ") + 1;
      lines = lines.substr(0, index);

      // Keep reading until error, it will get rid of spaces automatically
      std::stringstream ss(lines);
      while (ss.good()) {
        // Only add one to row if we are in the first column,
        // the size is fixed so there won't be a row with a different number of
        // orbs
        if (row == 0)
          column++;
        // Read it out as a number
        int a = 0;
        ss >> a;

        // Convert int into orbs
        board[currIndex] = Orb(a);
        currIndex++;
      }
      row++;
    }

    Configuration::shared().config(row, column, minErase);
    boardFile.close();
    return board;
  }

  /// Read a board from a string
  void setBoardFrom(const std::string &board) {
    // This is just a string with the board
    int size = board.length();
    // It is just a string so must be fixed size
    if (size == 20) // 5x4
    {
      row = 4;
      column = 5;
    } else if (size == 30) // 6x5
    {
      row = 5;
      column = 6;
    } else if (size == 42) // 7x6
    {
      row = 6;
      column = 7;
    }

    Configuration::shared().config(row, column, minErase);

    // Read from a string
    Board currBoard;
    currBoard.fill(pad::unknown);

    for (int i = 0; i < size; i++) {
      char orb = board[i];

      // Check if it is a number between 1 and 9
      if (orb >= '0' && orb <= '9') {
        currBoard[i] = pad::orbs(orb - '0');
      }

      // Check if it is a letter (RBGLDH)
      for (int k = 0; k < pad::ORB_COUNT; k++) {
        if (pad::ORB_SIMULATION_NAMES[k].c_str()[0] == orb) {
          currBoard[i] = Orb(k);
          break;
        }
      }
    }

    this->board = PBoard(currBoard);
  }

public:
  /// This is the original board
  PBoard board;

  PSolverSAT(const std::string &filePath, int minErase, int steps, int size) {
    this->minErase = minErase;
    this->steps = steps;
    this->size = size;

    if (filePath.find(".txt") != std::string::npos) {
      auto currBoard = readBoard(filePath);
      board = PBoard(currBoard);
    } else {
      setBoardFrom(filePath);
    }
  }

  /// Solve the current board
  std::vector<Route> solve() {
    std::vector<Route> routes;
    routes.reserve(5);

    return routes;
  }
};

#endif
