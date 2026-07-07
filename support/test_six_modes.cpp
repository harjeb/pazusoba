#include <pazusoba/core.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int apply_move(int pos, char move, int cols) {
    switch (move) {
        case 'U':
            return pos - cols;
        case 'D':
            return pos + cols;
        case 'L':
            return pos - 1;
        case 'R':
            return pos + 1;
        case 'Q':
            return pos - cols - 1;
        case 'E':
            return pos - cols + 1;
        case 'Z':
            return pos + cols - 1;
        case 'C':
            return pos + cols + 1;
        default:
            return pos;
    }
}

bool replay_route(const std::string& input,
                  int rows,
                  int cols,
                  const pazusoba::shape_result& result,
                  const std::array<bool, MAX_BOARD_LENGTH>& blocked) {
    if (!result.success)
        return false;
    if (result.start < 0 || result.start >= rows * cols || blocked[result.start])
        return false;

    std::string board = input;
    int curr = result.start;
    for (char move : result.route) {
        int next = apply_move(curr, move, cols);
        if (next < 0 || next >= rows * cols || blocked[next])
            return false;
        int dr = std::abs(next / cols - curr / cols);
        int dc = std::abs(next % cols - curr % cols);
        if ((dr == 0 && dc == 0) || dr > 1 || dc > 1)
            return false;
        std::swap(board[curr], board[next]);
        curr = next;
    }

    return board == result.final_board && result.steps == (int)result.route.size();
}

void test_blocked_expand() {
    pazusoba::solver solver;
    solver.set_board("DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD");
    int blocked[] = {1, 6, 6, -1, 100};
    solver.set_blocked(blocked, 5);

    std::vector<pazusoba::state> states(4);
    pazusoba::state top_left;
    top_left.curr = 0;
    top_left.prev = 0;
    top_left.begin = 0;
    solver.expand(solver.board(), top_left, states, 0);
    int valid = 0;
    for (const auto& s : states) {
        if (s.score != MIN_STATE_SCORE)
            valid++;
    }
    assert(valid == 0);
}

void test_7x6_board() {
    pazusoba::solver solver;
    solver.set_board("RBGDLHRBGDLHRBGDLHRBGDLHRBGDLHRBGDLHRBGDLH");
    assert(solver.board_size() == 42);
    assert(solver.row() == 6);
    assert(solver.column() == 7);
}

void test_diagonal_expand() {
    pazusoba::solver solver;
    solver.set_board("DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD");
    solver.set_diagonal(true);

    std::vector<pazusoba::state> states(8);
    pazusoba::state center;
    center.curr = 7;
    center.prev = 7;
    center.begin = 7;
    solver.expand(solver.board(), center, states, 0);

    int valid = 0;
    for (const auto& s : states) {
        if (s.score != MIN_STATE_SCORE)
            valid++;
    }
    assert(valid == 8);
}

void test_connected_orb_multicolor() {
    pazusoba::solver solver;
    solver.set_board("RRRRGGGGBBLDHLBDHBLDRHBLDRHBLD");
    pazusoba::profile profile;
    profile.name = pazusoba::connected_orb;
    profile.target = 4;
    profile.orbs[1] = true;  // R
    profile.orbs[3] = true;  // G
    solver.set_profiles(&profile, 1);

    auto board = solver.board();
    pazusoba::state state;
    solver.evaluate(board, state);
    assert(state.goal);
}

void test_target_combo_multicolor_requirement() {
    pazusoba::solver solver;
    solver.set_board("RRRBBBGGGLLLDDDHHHRRBBGGLDDHHL");
    pazusoba::profile profile;
    profile.name = pazusoba::target_combo;
    profile.target = -1;
    profile.colour_target = 2;
    profile.orbs[1] = true;  // R
    profile.orbs[3] = true;  // G
    solver.set_profiles(&profile, 1);

    auto board = solver.board();
    pazusoba::state state;
    solver.evaluate(board, state);
    assert(state.goal);
    assert(state.score > 0);
}

void test_shape(int kind, const std::string& board, int rows, int cols) {
    pazusoba::shape_request request;
    request.shape = kind;
    request.mode = pazusoba::color_prefer;
    request.colors[1] = true;  // R
    request.colors[3] = true;  // G

    std::array<bool, MAX_BOARD_LENGTH> blocked{};
    blocked.fill(false);
    auto result = pazusoba::solve_shape(board, rows, cols, request, blocked);
    if (!result.success) {
        std::cerr << "shape " << kind << " failed: " << result.note << "\n";
    }
    assert(result.success);
    assert(replay_route(board, rows, cols, result, blocked));
    assert(pazusoba::board_has_shape(result.final_board, rows, cols, kind));
    assert(result.color == 1 || result.color == 3);
}

void test_shape_blocked() {
    std::string board = "RRHRRBLRRBGDLHDRHBBGRGDDLRGBHL";
    pazusoba::shape_request request;
    request.shape = pazusoba::shape_cross;
    request.mode = pazusoba::color_only;
    request.colors[1] = true;

    std::array<bool, MAX_BOARD_LENGTH> blocked{};
    blocked.fill(false);
    blocked[0] = true;
    blocked[5] = true;

    auto result = pazusoba::solve_shape(board, 5, 6, request, blocked);
    assert(result.success);
    assert(replay_route(board, 5, 6, result, blocked));
    assert(pazusoba::board_has_shape(result.final_board, 5, 6, pazusoba::shape_cross));
}

void test_shape_diagonal() {
    std::string board = "RRHRRBLRRBGDLHDRHBBGRGDDLRGBHL";
    pazusoba::shape_request request;
    request.shape = pazusoba::shape_full_column;
    request.mode = pazusoba::color_prefer;
    request.allow_diagonal = true;
    request.colors[1] = true;
    request.colors[3] = true;

    std::array<bool, MAX_BOARD_LENGTH> blocked{};
    blocked.fill(false);
    auto result = pazusoba::solve_shape(board, 5, 6, request, blocked);
    assert(result.success);
    assert(replay_route(board, 5, 6, result, blocked));
    assert(pazusoba::board_has_shape(result.final_board, 5, 6, pazusoba::shape_full_column));
}

}  // namespace

int main() {
    test_blocked_expand();
    test_7x6_board();
    test_diagonal_expand();
    test_connected_orb_multicolor();
    test_target_combo_multicolor_requirement();

    std::string board_6x5 = "RRHRRBLRRBGDLHDRHBBGRGDDLRGBHL";
    test_shape(pazusoba::shape_3x3_square, board_6x5, 5, 6);
    test_shape(pazusoba::shape_cross, board_6x5, 5, 6);
    test_shape(pazusoba::shape_full_row, board_6x5, 5, 6);
    test_shape(pazusoba::shape_full_column, board_6x5, 5, 6);
    test_shape_blocked();
    test_shape_diagonal();

    std::string board_7x6 = "RBGDLHRBGDLHRBGDLHRBGDLHRBGDLHRBGDLHRBGDLH";
    test_shape(pazusoba::shape_cross, board_7x6, 6, 7);
    test_shape(pazusoba::shape_full_row, board_7x6, 6, 7);
    test_shape(pazusoba::shape_full_column, board_7x6, 6, 7);

    std::cout << "six modes tests passed\n";
    return 0;
}
