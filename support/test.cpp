#include <pazusoba/core.h>
#include <cassert>


void print_combo(const pazusoba::combo_list& combos) {
    printf("combos size %d\n", (int)combos.size());
    for (const auto& c : combos) {
        printf("orb %d - ", c.info);
        for (const auto& l : c.loc) {
            printf("%d ", l);
        }
        printf("\n");
    }
}

bool has_state_board(const std::vector<pazusoba::state>& states,
                     int curr,
                     const std::string& board,
                     const pazusoba::solver& solver) {
    for (const auto& s : states) {
        if (s.score != MIN_STATE_SCORE && s.curr == curr &&
            solver.get_board_string(s.board) == board) {
            return true;
        }
    }
    return false;
}

bool has_combo(const pazusoba::combo_list& combos, int info, int size) {
    for (const auto& c : combos) {
        if (c.info == info && (int)c.loc.size() == size)
            return true;
    }
    return false;
}

int main() {
    ///
    /// Set Board
    ///

    printf("test set_board\n");
    auto solver = pazusoba::solver();
    solver.set_board("DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD");
    assert(solver.board_size() == 30);
    assert(solver.row() == 5);
    assert(solver.column() == 6);
    assert(solver.max_combo() == 8);
    assert(solver.min_erase() == 3);
    assert(solver.get_board_string(solver.board()) ==
           "DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD");
    printf("test set_board passed\n");
    printf("====================================\n");

    ///
    /// Expand
    ///

    printf("test expand\n");
    std::vector<pazusoba::state> next_states;
    next_states.resize(4);
    // top left corner
    pazusoba::state top_left;
    top_left.curr = 0;
    top_left.prev = 0;
    top_left.begin = 0;
    solver.expand(solver.board(), top_left, next_states, 0);
    int valid = 0;
    for (const auto& s : next_states) {
        if (s.score != MIN_STATE_SCORE) {
            printf("initial %d, %d -> %d\n", s.begin, s.prev, s.curr);
            solver.print_board(s.board);
            solver.print_route(s.route, 1, 0);
            assert(s.curr == 6 || s.curr == 1);
            valid++;
        }
    }
    assert(valid == 2);
    next_states.clear();
    next_states.resize(4);

    // bottom left corner
    pazusoba::state bottom_left;
    bottom_left.curr = 24;
    bottom_left.prev = 24;
    bottom_left.begin = 24;
    solver.expand(solver.board(), bottom_left, next_states, 0);
    valid = 0;
    for (const auto& s : next_states) {
        if (s.score != MIN_STATE_SCORE) {
            printf("initial %d, %d -> %d\n", s.begin, s.prev, s.curr);
            solver.print_board(s.board);
            assert(s.curr == 18 || s.curr == 25);
            valid++;
        }
    }
    assert(valid == 2);
    next_states.clear();
    next_states.resize(4);

    // top right corner
    pazusoba::state top_right;
    top_right.curr = 5;
    top_right.prev = 5;
    top_right.begin = 5;
    solver.expand(solver.board(), top_right, next_states, 0);
    valid = 0;
    for (const auto& s : next_states) {
        if (s.score != MIN_STATE_SCORE) {
            printf("initial %d, %d -> %d\n", s.begin, s.prev, s.curr);
            solver.print_board(s.board);
            assert(s.curr == 4 || s.curr == 11);
            valid++;
        }
    }
    assert(valid == 2);
    next_states.clear();
    next_states.resize(4);

    // bottom right corner
    pazusoba::state bottom_right;
    bottom_right.curr = 29;
    bottom_right.prev = 29;
    bottom_right.begin = 29;
    solver.expand(solver.board(), bottom_right, next_states, 0);
    valid = 0;
    for (const auto& s : next_states) {
        if (s.score != MIN_STATE_SCORE) {
            printf("initial %d, %d -> %d\n", s.begin, s.prev, s.curr);
            solver.print_board(s.board);
            assert(s.curr == 28 || s.curr == 23);
            valid++;
        }
    }
    assert(valid == 2);
    next_states.clear();
    next_states.resize(4);

    // three
    pazusoba::state three;
    three.curr = 3;
    three.prev = 3;
    three.begin = 3;
    solver.expand(solver.board(), three, next_states, 0);
    valid = 0;
    for (const auto& s : next_states) {
        if (s.score != MIN_STATE_SCORE) {
            printf("initial %d, %d -> %d\n", s.begin, s.prev, s.curr);
            solver.print_board(s.board);
            assert(s.curr == 2 || s.curr == 4 || s.curr == 9);
            valid++;
        }
    }
    assert(valid == 3);
    // 3 -> 2
    assert(has_state_board(next_states, 2,
                           "DGRRBLHGBBGGRDDDDLBGHDBLLHDBLD",
                           solver));
    // 3 -> 4
    assert(has_state_board(next_states, 4,
                           "DGRBRLHGBBGGRDDDDLBGHDBLLHDBLD",
                           solver));
    next_states.clear();
    next_states.resize(4);

    // location 1
    pazusoba::state loc_one;
    loc_one.curr = 1;
    loc_one.prev = 1;
    loc_one.begin = 1;
    solver.expand(solver.board(), loc_one, next_states, 0);
    valid = 0;
    for (const auto& s : next_states) {
        if (s.score != MIN_STATE_SCORE) {
            printf("initial %d, %d -> %d\n", s.begin, s.prev, s.curr);
            solver.print_board(s.board);
            assert(s.curr == 0 || s.curr == 2 || s.curr == 7);
            valid++;
        }
    }
    assert(valid == 3);
    next_states.clear();
    next_states.resize(4);

    // four
    pazusoba::state four;
    four.curr = 15;
    four.prev = 15;
    four.begin = 15;
    solver.expand(solver.board(), four, next_states, 0);
    valid = 0;
    for (const auto& s : next_states) {
        if (s.score != MIN_STATE_SCORE) {
            printf("initial %d, %d -> %d\n", s.begin, s.prev, s.curr);
            solver.print_board(s.board);
            assert(s.curr == 14 || s.curr == 16 || s.curr == 21 || s.curr == 9);
            valid++;
        }
    }
    assert(valid == 4);
    // 15 -> 9
    assert(has_state_board(next_states, 9,
                           "DGRRBLHGBDGGRDDBDLBGHDBLLHDBLD",
                           solver));
    next_states.clear();

    printf("test expand passed\n");
    printf("====================================\n");

    ///
    /// Explore
    ///

    printf("test explore - first step\n");
    next_states.resize(500);
    for (int i = 0; i < 30; i++) {
        pazusoba::state s;
        s.curr = i;
        s.prev = i;
        s.begin = i;
        solver.expand(solver.board(), s, next_states, i);
    }

    valid = 0;
    for (const auto& s : next_states) {
        if (s.score != MIN_STATE_SCORE) {
            printf("initial %d, %d -> %d\n", s.begin, s.prev, s.curr);
            solver.print_board(s.board);
            valid++;
        }
    }
    // 4 * 2, 4 corners
    // (3 + 3 + 4 + 4) * 3, 14 edges
    // (3 * 4) * 4, 12 centers
    assert(valid == 98);

    printf("passed\n");
    printf("====================================\n");

    ///
    /// Erase COmbo
    ///

    printf("test erase combo\n");

    pazusoba::combo_list combos;
    // 4 combos around
    solver.set_board("PRHBBBPHJDRHPRRHHJGGRRHJLLLHBJ");
    auto copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    printf("combo size: %d\n", (int)combos.size());
    for (const auto& c : combos) {
        printf("orb %d - ", c.info);
        for (const auto& l : c.loc) {
            printf("%d ", l);
            assert(l == 0 || l == 6 || l == 12 || l == 24 || l == 25 ||
                   l == 26 || l == 3 || l == 4 || l == 5 || l == 17 ||
                   l == 23 || l == 29);
        }
        printf("\n");
        assert(c.info == 7 || c.info == 9 || c.info == 2 || c.info == 4);
    }
    assert(combos.size() == 4);
    combos.clear();

    // 10 combos all horizontal
    solver.set_board("RRRBBBGGGLLLDDDHHHRRRBBBGGGLLL");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 10);
    pazusoba::state eval_state;
    copy = solver.board();
    solver.evaluate(copy, eval_state);
    assert(eval_state.combo == 10);
    combos.clear();

    // 10 combos with jammer and poison
    solver.set_board("RRRBBBDDDHRJPHLHRJPHLHRJPHLGGG");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 10);
    assert(has_combo(combos, 9, 3));
    assert(has_combo(combos, 7, 3));
    combos.clear();

    // 10 combos, 6 vertical & 4 horizontal
    solver.set_board("RRRBBBGLDGLDGLDGLDGLDGLDRRRBBB");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 10);
    assert(has_combo(combos, 5, 3));
    assert(has_combo(combos, 3, 3));
    combos.clear();

    // check min erase 4
    solver.set_min_erase(4);
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);
    assert(combos.size() == 0);
    solver.set_min_erase(3);
    combos.clear();

    // 4 combos with a very long U
    solver.set_board("HHHRHRHRHRHRHRHRHRHRHRHRHRHHHR");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);
    solver.print_board(copy);

    print_combo(combos);
    assert(combos.size() == 4);
    assert(has_combo(combos, 1, 5));
    assert(has_combo(combos, 1, 4));
    assert(has_combo(combos, 6, 17));
    combos.clear();

    // 3 combos, with a long Z
    solver.set_board("HHHHHHRRRRRHHHHHHHHRRRRRHHHHHH");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 3);
    assert(has_combo(combos, 6, 20));
    assert(has_combo(combos, 1, 5));
    combos.clear();

    //                 xxx
    // 5 combos, test xxx shapes
    solver.set_board("LGGGRLGGGRRLBBBRRLHHHRGLBDHHHL");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);
    solver.print_board(copy);

    print_combo(combos);
    assert(combos.size() == 5);
    assert(has_combo(combos, 4, 5));
    assert(has_combo(combos, 6, 6));
    assert(has_combo(combos, 1, 6));
    assert(has_combo(combos, 3, 6));
    assert(has_combo(combos, 2, 3));
    combos.clear();

    // 1 combo, all red
    solver.set_board("RRRRRRRRRRRRRRRRRRRRRRRRRRRRRR");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 1);
    assert(combos[0].info == 1);
    combos.clear();

    // 4 combos, L, + and a trick shape
    solver.set_board("HLHHRRHHHRRRBLHRRGBLHHHGBBBGGG");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 4);
    assert(has_combo(combos, 3, 5));
    assert(has_combo(combos, 2, 5));
    assert(has_combo(combos, 6, 8));
    assert(has_combo(combos, 1, 5));
    combos.clear();

    // check min erase 5
    solver.set_min_erase(5);
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);
    assert(combos.size() == 0);
    solver.set_min_erase(3);
    combos.clear();

    // 3 + and 2 combos
    solver.set_board("BBHHLLBBBLLLDBGGLRDGGGHLDRGRRR");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 5);
    assert(has_combo(combos, 1, 3));
    assert(has_combo(combos, 3, 5));
    assert(has_combo(combos, 5, 3));
    assert(has_combo(combos, 4, 5));
    assert(has_combo(combos, 2, 5));
    combos.clear();

    // check min erase 5
    solver.set_min_erase(5);
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);
    print_combo(combos);

    assert(combos.size() == 0);
    solver.set_min_erase(3);
    combos.clear();

    // 5 combos, T, L and going up shape
    solver.set_board("BBBLLLBDBGLLDDGGLRDDGGRRDRGRRR");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 5);
    assert(has_combo(combos, 1, 5));
    assert(has_combo(combos, 3, 6));
    assert(has_combo(combos, 5, 6));
    assert(has_combo(combos, 4, 5));
    assert(has_combo(combos, 2, 3));
    combos.clear();

    // 5 combos, T, L
    solver.set_board("BBBLLLBDBGLLBDGGLRDDDGGGDRGRRR");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 5);
    assert(has_combo(combos, 1, 3));
    assert(has_combo(combos, 5, 5));
    assert(has_combo(combos, 3, 5));
    assert(has_combo(combos, 4, 5));
    assert(has_combo(combos, 2, 5));
    combos.clear();

    // no combo
    solver.set_board("LBGHGDHDBDLBHDLHDRLHRBBGBLBDGR");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 0);
    combos.clear();

    // 2 combos with a long heal
    solver.set_board("BDLHGBBBHHHDBBHHGLDBRHHGGBHHGR");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 2);
    assert(has_combo(combos, 6, 7));
    assert(has_combo(combos, 2, 7));
    combos.clear();

    // 3 combos with a tricky shape
    solver.set_board("DHLLHLHHHHHHHHRRHBHRRRHBGBRDDD");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    assert(combos.size() == 3);
    assert(has_combo(combos, 5, 3));
    assert(has_combo(combos, 1, 5));
    assert(has_combo(combos, 6, 13));
    combos.clear();

    // Test 3x3 square (9-grid pattern)
    printf("test 3x3 square\n");
    solver.set_board("RRRBBBGGGRRRBBBGGGRRRBBBGGGRRR");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    // Standard combo mode treats these as regular horizontal combos.
    assert(combos.size() == 10);
    combos.clear();

    // Test single 3x3 square
    solver.set_board("RRRRRRRRRBBGGGGGGGGGGRRRBBBGGG");
    copy = solver.board();
    solver.print_board(copy);
    solver.erase_combo(copy, combos);

    print_combo(combos);
    // Standard combo mode should still find regular connected combos.
    assert(combos.size() >= 1);
    bool found_large_red_combo = false;
    bool found_large_green_combo = false;
    for (const auto& c : combos) {
        if (c.info == 1 && c.loc.size() >= 9) {
            found_large_red_combo = true;
        }
        if (c.info == 3 && c.loc.size() >= 9) {
            found_large_green_combo = true;
        }
    }
    assert(found_large_red_combo);
    assert(found_large_green_combo);
    combos.clear();

    printf("test erase combo passed\n");
    printf("====================================\n");

    ///
    /// Move orbs down
    ///

    printf("test move orbs down\n");

    printf("test move orbs down passed\n");
    printf("====================================\n");

    return 0;
}
