#pragma once
#ifndef _PAZUSOBA_SHAPE_H_
#define _PAZUSOBA_SHAPE_H_

#include "pazusoba.h"
#include <array>
#include <string>
#include <vector>

namespace pazusoba {

enum shape_kind {
    shape_3x3_square = 0,
    shape_cross,
    shape_full_row,
    shape_full_column,
};

enum color_mode {
    color_auto = 0,
    color_only,
    color_prefer,
};

struct shape_request {
    int shape = shape_3x3_square;
    int mode = color_auto;
    bool colors[ORB_COUNT]{false};
    bool strict_isolation = false;
    bool allow_diagonal = false;
};

struct shape_result {
    bool success = false;
    int steps = -1;
    int start = -1;
    int color = 0;
    int combo = -1;
    int max_combo = -1;
    std::string route;
    std::string final_board;
    std::string note;
};

struct shape_template {
    int kind = shape_3x3_square;
    std::vector<std::vector<int>> placements(int rows, int cols) const;
    int orb_count(int rows, int cols) const;
};

shape_template make_shape_template(int kind);

bool board_has_shape(const std::string& board,
                     int rows,
                     int cols,
                     int kind,
                     char* color = nullptr);

shape_result solve_shape(const std::string& board,
                         int rows,
                         int cols,
                         const shape_request& request);

shape_result solve_shape(const std::string& board,
                         int rows,
                         int cols,
                         const shape_request& request,
                         const std::array<bool, MAX_BOARD_LENGTH>& blocked);

}  // namespace pazusoba

#endif
