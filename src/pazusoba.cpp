// pazusoba.cpp
// Try to improve the performance of the solver while being flexible enough

// Compile with
// mac: clang++ -std=c++11 -fopenmp -O2 pazusoba.cpp -o pazusoba
// windows: g++ -std=c++11 -fopenmp -O2 pazusoba.cpp -o pazusoba

#include <pazusoba/core.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <thread>
#include <vector>

namespace pazusoba {
state solver::adventure() {
    int REAL_BEAM_SIZE = BEAM_SIZE * 1.4;
    // setup the state, non blocking
    std::vector<state> look;
    look.reserve(REAL_BEAM_SIZE);
    // insert to temp, sort and copy back to look
    std::vector<state> temp;
    temp.resize(REAL_BEAM_SIZE * 3);
    // TODO: using array can definitely things a lot because the vector needs to
    // write a lot of useless data before using it, reverse is better but the
    // address calculation can be tricky

    state best_state;
    bool found_max_combo = false;

    // assign all possible states to look
    for (int i = 0; i < BOARD_SIZE; ++i) {
        state new_state;
        new_state.curr = i;
        new_state.prev = i;
        new_state.begin = i;
        new_state.score = MIN_STATE_SCORE + 1;
        look.push_back(new_state);
    }

    // setup threading
    int processor_count = std::thread::hardware_concurrency();
    // unsigned int processor_count = 1;
    std::vector<std::thread> threads;
    threads.reserve(processor_count);

    int stop_count = 0;

    // beam search with openmp
    for (int i = 0; i < SEARCH_DEPTH; i++) {
        if (found_max_combo)
            break;

        int look_size = look.size();
        DEBUG_PRINT("Depth %d - size %d\n", i + 1, look_size);
        int look_size_thread = look_size / processor_count;

        // #pragma omp parallel for
        for (int thread_num = 0; thread_num < processor_count; thread_num++) {
            threads.emplace_back([&, thread_num, look_size_thread] {
                int start_index = thread_num * (look_size_thread);
                int end_index = start_index + look_size_thread;
                for (int j = start_index; j < end_index; j++) {
                    if (found_max_combo)
                        continue;  // early stop

                    const state& curr = look[j];

                    if (curr.goal) {
                        best_state = curr;
                        found_max_combo = true;
                        continue;
                    }

                    expand(curr.board, curr, temp, j);
                }
            });
        }

        for (auto& t : threads)
            t.join();
        threads.clear();

        // break out as soon as max combo or target is found
        // TODO: this should be the target
        if (found_max_combo)
            break;

        // DEBUG_PRINT("%d... ", i);
        DEBUG_PRINT("Depth %d - sorting\n", i + 1);

        // sorting
        auto begin = temp.begin();
        auto end = temp.end();
        std::sort(begin, end, std::greater<state>());

        for (int i = 0; i < 5; i++) {
            // print_state(temp[i]);
            DEBUG_PRINT("combo %d\n", temp[i].combo);
        }

        // (end - begin) gets the size of the vector, divide by 3 to get the
        // number of states we consider in the next step

        look.clear();
        // we need to filter out the states that are already visited
        int index = 0;
        for (int j = 0; j < REAL_BEAM_SIZE; j++, index++) {
            const auto& curr = temp[j];
            if (VISITED[curr.hash]) {
                index--;
            } else {
                VISITED[curr.hash] = true;
                if (curr.score > best_state.score) {
                    best_state = curr;
                    stop_count = 0;
                }

                // break if empty boards are hit
                if (curr.score == MIN_STATE_SCORE) {
                    break;
                }
                look.push_back(curr);
            }
        }

        // std::copy(begin, begin + (end - begin) / 3, look.begin());
        stop_count++;
        if (stop_count > STOP_THRESHOLD) {
            break;
        }
    }

    // print_state(best_state);
    return best_state;
}  // namespace pazusoba

void solver::expand(const game_board& board,
                    const state& current,
                    std::vector<state>& states,
                    const int loc) {
    int count = ALLOW_DIAGONAL ? DIRECTION_COUNT : 4;

    auto prev = current.prev;
    auto curr = current.curr;
    auto step = current.step;
    for (int i = 0; i < count; i++) {
        // this is set from parse_args()
        int adjustments = DIRECTION_ADJUSTMENTS[i];
        tiny next = curr + adjustments;
        // todo: right edge can be checked before the calculation
        if (next == prev)
            continue;  // invalid, same position
        if (next - curr == 1 && next % COLUMN == 0)
            continue;  // invalid, on the right edge
        if (curr - next == 1 && curr % COLUMN == 0)
            continue;  // invalid, on the left edge
        if (next >= BOARD_SIZE)
            continue;  // invalid, out of bound

        state new_state;
        new_state.step = step + 1;
        new_state.curr = next;
        new_state.prev = curr;
        new_state.begin = current.begin;
        new_state.route = current.route;

        // insert to the route
        int route_index = new_state.step / ROUTE_PER_LIST;
        if (new_state.step % ROUTE_PER_LIST == 0)
            route_index--;  // the last one in the previous number
        new_state.route[route_index] = new_state.route[route_index] << 3 | i;

        if (step == 0)
            new_state.board = BOARD;
        else
            new_state.board = board;

        // swap the board
        auto& new_board = new_state.board;
        auto temp = new_board[curr];
        new_board[curr] = new_board[next];
        new_board[next] = temp;

        // calculate the hash
        new_state.hash = hash::pazusoba_hash(new_board.data(), new_state.prev);

        // backup the board
        evaluate(new_board, new_state);

        // insert to the states
        if (step == 0)
            states[loc * 4 + i] = new_state;
        else
            states[loc * 3 + i] = new_state;
    }
}

void solver::evaluate(game_board& board, state& new_state) {
    short int score = 0;
    // TODO: should this be after??
    // scan the board to get the distance between each orb
    orb_distance distance[ORB_COUNT];
    for (int i = 0; i < BOARD_SIZE; i++) {
        auto& orb = board[i];
        // 0 to 5 only for 6x5, 6 to 11 will convert to 0 to 5
        int loc = i % COLUMN;
        if (loc > distance[orb].max)
            distance[orb].max = loc;
        else if (loc < distance[orb].min)
            distance[orb].min = loc;
    }

    for (int i = 0; i < ORB_COUNT; i++) {
        auto& dist = distance[i];
        score -= (dist.max - dist.min);
    }

    // erase the board and find out the combo number
    combo_list list;  // TODO: 515ms here, destructor is slow

    int combo = 0;
    int move_count = 0;
    game_board copy = board;
    game_board prev_board = board;  // 用于检测棋盘状态是否重复
    const int MAX_ELIMINATION_ROUNDS = 20;  // 防止无限循环的保护机制

    DEBUG_PRINT("=== Starting combo elimination loop ===\n");

    while (move_count < MAX_ELIMINATION_ROUNDS) {
        DEBUG_PRINT("Elimination round %d:\n", move_count + 1);
        list.clear();  // 确保列表在每次调用前是空的
        erase_combo(copy, list);
        int combo_count = list.size();
        DEBUG_PRINT("Combo count this round: %d\n", combo_count);

        // Check if there are more combo
        if (combo_count > 0) {
            // 修复：combo应该是连锁的轮数，而不是累加每轮的combo数量
            combo = move_count + 1;  // combo等于当前轮数
            DEBUG_PRINT("Current combo chain length: %d\n", combo);

            // 保存当前棋盘状态
            prev_board = copy;
            move_orbs_down(copy);

            // 检测棋盘状态是否重复（死循环检测）
            bool board_unchanged = true;
            for (int i = 0; i < BOARD_SIZE; i++) {
                if (copy[i] != prev_board[i]) {
                    board_unchanged = false;
                    break;
                }
            }

            if (board_unchanged) {
                DEBUG_PRINT("WARNING: Board state unchanged after move_orbs_down, breaking to prevent infinite loop\n");
                break;
            }

            move_count++;
            DEBUG_PRINT("Move count: %d\n", move_count);
        } else {
            DEBUG_PRINT("No new combos, breaking loop\n");
            break;
        }
    }

    if (move_count >= MAX_ELIMINATION_ROUNDS) {
        DEBUG_PRINT("WARNING: Maximum elimination rounds reached, breaking to prevent infinite loop\n");
    }

    DEBUG_PRINT("Final combo count: %d, Total rounds: %d\n", combo, move_count);

    // track if all goals are reached
    int goal = 0;
    for (int i = 0; i < PROFILE_COUNT; i++) {
        const auto& profile = PROFILES[i];
        switch (profile.name) {
            case target_combo: {
                int target = profile.target;
                if (target == -1) {
                    // max combo
                    score += combo * 20;
                    if (combo == MAX_COMBO)
                        goal++;
                } else {
                    // only do max target combo
                    if (combo < target)
                        score -= (7 - target) * 30;
                    if (combo == target)
                        score += 50;
                    else if (target > 7)
                        score -= 50;

                    if (combo == target)
                        goal++;
                }
            } break;

            case colour: {
                int colour_counter[ORB_COUNT]{0};
                for (const auto& c : list) {
                    colour_counter[c.info]++;
                }

                bool has_all_target_colours = true;
                for (int j = 0; j < ORB_COUNT; j++) {
                    // this orb should be included
                    if (profile.orbs[j]) {
                        // just add a tiny score, don't do too much
                        if (colour_counter[j] == 0)
                            has_all_target_colours = false;
                        else
                            score += 2;
                    }
                }

                if (has_all_target_colours)
                    goal++;
            } break;

            case colour_combo: {
                int colour_counter[ORB_COUNT]{0};
                for (const auto& c : list) {
                    colour_counter[c.info]++;
                }

                bool fulfilled = true;
                for (int j = 0; j < ORB_COUNT; j++) {
                    // this orb should be included
                    if (profile.orbs[j]) {
                        int colour_combo = colour_counter[j];
                        // just add a tiny score, don't do too much
                        if (colour_combo == 0)
                            fulfilled = false;
                        else if (colour_combo >= profile.target)
                            score += 2;
                    }
                }

                if (fulfilled)
                    goal++;
            } break;

            case connected_orb: {
                int target = profile.target;
                bool fulfilled = false;

                for (const auto& c : list) {
                    int connected_count = c.loc.size();
                    if (ORB_COUNTER[c.info] >= target) {
                        if (connected_count < target) {
                            score += (connected_count - MIN_ERASE) * 10;
                        } else if (connected_count == target) {
                            // fulfilled = true;
                            score += 50;
                        } else {
                            score -= (connected_count - target) * 50;
                        }
                    }
                }

                score += combo * 20;

                if (fulfilled)
                    goal++;
            } break;

            case orb_remaining: {
                int remaining = 0;
                for (int j = 0; j < BOARD_SIZE; j++) {
                    if (copy[j] > 0)
                        remaining++;
                }

                if (remaining <= profile.target)
                    goal++;
                score -= remaining * 10;
            } break;

            case shape_L: {
                for (const auto& c : list) {
                    if (profile.orbs[c.info] && ORB_COUNTER[c.info] >= 5) {
                        int size = c.loc.size();
                        if (size == 5) {
                            // some score for connecting more orbs
                            // check if it is L shape
                            std::map<int, int> vertical;
                            std::map<int, int> horizontal;
                            int bigFirst = -1;
                            int bigSecond = -1;

                            // Collect info
                            for (const auto& loc : c.loc) {
                                int x = loc % COLUMN;
                                int y = loc / COLUMN;
                                vertical[x]++;
                                horizontal[y]++;

                                // Track the largest number
                                if (vertical[x] >= 3)
                                    bigFirst = x;
                                if (horizontal[y] >= 3)
                                    bigSecond = y;
                            }

                            // This is the center point
                            if (bigFirst > -1 && bigSecond > -1) {
                                int counter = 0;
                                // Check if bigFirst -2 or +2 exists
                                if (vertical[bigFirst - 2] > 0 ||
                                    vertical[bigFirst + 2] > 0)
                                    counter++;
                                // Same for bigSecond
                                if (horizontal[bigSecond - 2] > 0 ||
                                    horizontal[bigSecond + 2] > 0)
                                    counter++;

                                if (counter == 2)
                                    score += 50;
                            }
                        } else if (size > 3) {
                            score += 10;
                        }
                    }
                }

                // consider combo here as well
                score += combo * 20;
            } break;

            case shape_plus: {
                for (const auto& c : list) {
                    if (profile.orbs[c.info] && ORB_COUNTER[c.info] >= 5) {
                        int size = c.loc.size();
                        if (size <= 5)
                            score += (size - MIN_ERASE) * 10;

                        // some score for connecting more orbs
                        // check if it is L shape
                        std::map<int, int> vertical;
                        std::map<int, int> horizontal;
                        int bigFirst = -1;
                        int bigSecond = -1;

                        // Collect info
                        for (const auto& loc : c.loc) {
                            int x = loc % COLUMN;
                            int y = loc / COLUMN;
                            vertical[x]++;
                            horizontal[y]++;

                            // Track the largest number
                            if (vertical[x] >= 3)
                                bigFirst = x;
                            if (horizontal[y] >= 3)
                                bigSecond = y;
                        }

                        // This is the center point
                        if (bigFirst > -1 && bigSecond > -1) {
                            int counter = 0;
                            // Check up down left right there is an orb around
                            // center orb
                            if (vertical[bigFirst - 1] > 0 &&
                                vertical[bigFirst + 1] > 0)
                                counter++;
                            if (horizontal[bigSecond - 1] > 0 &&
                                horizontal[bigSecond + 1] > 0)
                                counter++;

                            if (counter == 2)
                                score += 50;
                            if (counter == 1)
                                score += 10;
                        }
                    }
                }
            } break;

            case shape_square: {
                // FORCE 3x3 MODE: Massively prioritize 3x3 squares over everything else
                bool found_3x3 = false;
                printf("DEBUG: Checking %d combos for 3x3 squares\n", (int)list.size());
                for (const auto& c : list) {
                    printf("DEBUG: Combo of orb %d, size %d, orb_counter[%d]=%d\n", 
                           c.info, (int)c.loc.size(), c.info, ORB_COUNTER[c.info]);
                    if (profile.orbs[c.info] && ORB_COUNTER[c.info] >= 9) {
                        int size = c.loc.size();
                        if (size >= 9) {
                            // Check if it forms a 3x3 square
                            printf("DEBUG: Checking 3x3 for orb %d with size %d\n", c.info, size);
                            if (is_3x3_square(c.loc, COLUMN)) {
                                printf("DEBUG: FOUND 3x3 SQUARE! Orb %d\n", c.info);
                                score += 50000;  // MASSIVE score for 3x3
                                goal++;
                                found_3x3 = true;
                            } else {
                                printf("DEBUG: Not a 3x3 square\n");
                            }
                        }
                    }
                }
                
                // If we found a 3x3, ignore all other scoring
                if (found_3x3) {
                    new_state.combo = combo;
                    new_state.score = score;
                    new_state.goal = (goal > 0);
                    return;
                }
                
                // If no 3x3 found, look for potential 3x3 formation
                // Check for clusters of 6+ orbs that could become 3x3
                for (const auto& c : list) {
                    if (ORB_COUNTER[c.info] >= 9) {
                        int size = c.loc.size();
                        if (size >= 6) {
                            score += size * 20;  // Reward large clusters
                        }
                    }
                }
                
                // Check 3x3 potential for each color with 9+ orbs
                for (int orb_type = 1; orb_type < ORB_COUNT; orb_type++) {
                    if (ORB_COUNTER[orb_type] >= 9) {
                        // Analyze 3x3 potential for this orb type
                        for (int top_row = 0; top_row <= ROW - 3; top_row++) {
                            for (int top_col = 0; top_col <= COLUMN - 3; top_col++) {
                                int matching_orbs = 0;
                                
                                // Count matching orbs in this 3x3 area
                                for (int i = 0; i < 3; i++) {
                                    for (int j = 0; j < 3; j++) {
                                        int pos = (top_row + i) * COLUMN + (top_col + j);
                                        if (copy[pos] == orb_type) {
                                            matching_orbs++;
                                        }
                                    }
                                }
                                
                                // Score based on 3x3 potential
                                if (matching_orbs >= 6) {
                                    score += 1000;  // Very close to 3x3
                                } else if (matching_orbs >= 4) {
                                    score += 200;   // Good potential
                                } else if (matching_orbs >= 2) {
                                    score += 50;    // Some potential
                                }
                            }
                        }
                    }
                }
            } break;

            case shape_row: {
            } break;

            case shape_column: {
            } break;

            default: {
                printf("unknown profile %d\n", profile.name);
                exit(1);
            } break;
        }
    }

    new_state.combo = combo;
    new_state.score = score;

    if (goal == PROFILE_COUNT) {
        new_state.goal = true;
    }
}

void solver::erase_combo(game_board& board, combo_list& list) {
    DEBUG_PRINT("=== erase_combo called ===\n");
    visit_board visited_location{0};
    
    // First, check for 3x3 squares (9-grid pattern)
    check_3x3_squares(board, list, visited_location);
    
    // Then, check for regular combos
    for (int curr_index = BOARD_SIZE - 1; curr_index >= 0; curr_index--) {
        if (visited_location[curr_index])
            continue;  // already visited even if it is not erased

        auto orb = board[curr_index];
        if (orb == 0)
            continue;  // already erased

        combo c(orb);
        std::queue<int> visit_queue;
        visit_queue.emplace(curr_index);

        // start exploring until all connected orbs are visited
        int visit_count = 0;
        const int MAX_VISIT_COUNT = BOARD_SIZE * 2;  // 防止无限循环

        while (!visit_queue.empty() && visit_count < MAX_VISIT_COUNT) {
            int to_visit = visit_queue.front();
            visit_queue.pop();
            visit_count++;

            // number of connected orbs in all directions
            int counter[4]{0};

            // check all four directions
            for (int i = 0; i < 4; i++) {
                int direction = DIRECTION_ADJUSTMENTS[i];
                // this needs to be unsigned to avoid negatives
                tiny next = to_visit;
                int step_count = 0;
                const int MAX_STEPS = BOARD_SIZE;  // 防止单方向无限循环

                // going in that direction until a different orb is found
                while (step_count < MAX_STEPS) {
                    if (direction == -1 && next % COLUMN == 0)
                        break;  // invalid, on the left edge

                    next += direction;
                    step_count++;

                    if (direction == 1 && next % COLUMN == 0)
                        break;  // invalid, on the right edge
                    if (next >= BOARD_SIZE)
                        break;  // invalid, out of bound

                    if (board[next] == orb) {
                        // same colour
                        visited_location[next] = true;
                        counter[i]++;

                        // check if there are orbs in the different direction
                        for (int j = 0; j < 4; j++) {
                            if (i < 2 && j < 2)
                                continue;  // only search left & right
                            if (i >= 2 && j >= 2)
                                continue;  // only search up & down

                            int direction = DIRECTION_ADJUSTMENTS[j];
                            tiny nearby = next;
                            if (direction == -1 && nearby % COLUMN == 0)
                                continue;  // invalid, on the left edge

                            nearby += direction;

                            if (direction == 1 && nearby % COLUMN == 0)
                                continue;  // invalid, on the right edge
                            if (nearby >= BOARD_SIZE)
                                continue;  // invalid, out of bound
                            if (visited_location[nearby])
                                continue;  // invalid, already visited

                            // same orb in different direction, should visit
                            if (board[nearby] == orb) {
                                // check next first before nearby
                                visit_queue.emplace(next);
                                visit_queue.emplace(nearby);
                            }
                        }
                    } else {
                        break;  // different colour
                    }
                }

                if (step_count >= MAX_STEPS) {
                    DEBUG_PRINT("WARNING: Maximum steps reached in direction %d, breaking to prevent infinite loop\n", i);
                }
            }

            if (visit_count >= MAX_VISIT_COUNT) {
                DEBUG_PRINT("WARNING: Maximum visit count reached, breaking to prevent infinite loop\n");
            }

            // only 2 same orbs are needed to make 3 in a row
            if (counter[0] + counter[1] >= 2) {
                c.loc.insert(to_visit);
                board[to_visit] = 0;
                // up & down
                for (int i = -counter[0]; i <= counter[1]; i++) {
                    if (i == 0)
                        continue;  // this is the source orb itself
                    // convert index to location, -1 moves -6 for 6x5
                    auto index = to_visit + i * COLUMN;
                    c.loc.insert(index);
                    board[index] = 0;
                }
            }

            if (counter[2] + counter[3] >= 2) {
                c.loc.insert(to_visit);
                board[to_visit] = 0;
                // left & right
                for (int i = -counter[2]; i <= counter[3]; i++) {
                    if (i == 0)
                        continue;  // this is the source orb itself
                    auto index = to_visit + i;
                    c.loc.insert(index);
                    board[index] = 0;
                }
            }
        }

        // add this combo to the list
        if ((int)c.loc.size() >= MIN_ERASE) {
            list.push_back(c);
            DEBUG_PRINT("Regular combo added: size %d, color %d\n", (int)c.loc.size(), c.info);
        }
    }
    
    DEBUG_PRINT("Total combos found in erase_combo: %d\n", (int)list.size());
}

void solver::move_orbs_down(game_board& board) {
    // TODO: maybe should taking min erase into account
    // because it is impossible to erase only one orb
    for (int i = 0; i < COLUMN; ++i) {
        int emptyIndex = -1;
        // signed type is needed or otherwise, j >= won't terminate at all
        // because after -1 is the max value again
        for (int j = ROW - 1; j >= 0; --j) {
            int index = INDEX_OF(j, i);
            orb o = board[index];
            if (o == 0) {
                // Don't override empty index if available
                if (emptyIndex == -1)
                    emptyIndex = j;
            } else if (emptyIndex != -1) {
                // replace last known empty index
                // and replace it with current index
                board[INDEX_OF(emptyIndex, i)] = o;
                board[index] = 0;
                // simply move it up from last index
                --emptyIndex;
            }
        }
    }
}

int solver::calc_max_combo(const orb_list& counter,
                           const int size,
                           const int min_erase) const {
    // at least one combo when the board has only one orb
    int max_combo = 0;
    int threshold = size / 2;
    for (const auto& count : counter) {
        int combo = count / min_erase;
        // based on my experience, it is not possible to do more combo
        // if one colour has more than half the board
        // the max combo needs to be reduced by 2 times
        // RRRRRRRRRRRRRRRRRRRRRRRRGGGBBB can do max 4 combos naively
        // this is because R is taking up too much spaces
        // MAX_COMBO might not be 100% correct but it's a good reference
        if (count > threshold) {
            int extra_combo = (count - threshold) * 2 / min_erase;
            combo -= extra_combo;
        }
        max_combo += combo;
    }

    if (max_combo == 0)
        return 1;
    return max_combo;
}

void solver::parse_args(int argc, char* argv[]) {
    if (argc <= 1)
        usage();

    // min_erase needs to know before parsing the board,
    // this is to calculate the max combo
    if (argc > 2) {
        int min_erase = atoi(argv[2]);
        set_min_erase(min_erase);
    }

    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            usage();
        } else {
            DEBUG_PRINT("=============== INFO ===============\n");
            auto board_string = argv[1];
            set_board(board_string);
        }
    }

    if (argc > 3) {
        int depth = atoi(argv[3]);
        set_search_depth(depth);
    }

    if (argc > 4) {
        int beam_size = atoi(argv[4]);
        set_beam_size(beam_size);
    }

    if (argc > 5) {
        if (strcmp(argv[5], "--diagonal") == 0 || strcmp(argv[5], "-d") == 0) {
            set_diagonal(true);
        }
    }

    print_board(BOARD);
    DEBUG_PRINT("board size: %d\n", BOARD_SIZE);
    DEBUG_PRINT("row x column: %d x %d\n", ROW, COLUMN);
    DEBUG_PRINT("min_erase: %d\n", MIN_ERASE);
    DEBUG_PRINT("max_combo: %d\n", MAX_COMBO);
    DEBUG_PRINT("search_depth: %d\n", SEARCH_DEPTH);
    DEBUG_PRINT("beam_size: %d\n", BEAM_SIZE);
    DEBUG_PRINT("diagonal_movement: %s\n", ALLOW_DIAGONAL ? "enabled" : "disabled");
    DEBUG_PRINT("====================================\n");
}

void solver::set_board(const char* board_string) {
    int board_size = strlen(board_string);

    // there are only 3 fixed size board -> 20, 30 or 42
    if (board_size > MAX_BOARD_LENGTH) {
        printf("Board string is too long\n");
        exit(1);
    } else if (board_size == 20) {
        ROW = 4;
        COLUMN = 5;
    } else if (board_size == 30) {
        ROW = 5;
        COLUMN = 6;
    } else if (board_size == 42) {
        ROW = 6;
        COLUMN = 7;
    } else {
        printf("Unsupported board size - %d\n", board_size);
        exit(1);
    }
    BOARD_SIZE = board_size;

    // set up DIRECTION_ADJUSTMENTS
    DIRECTION_ADJUSTMENTS[0] = -COLUMN;
    DIRECTION_ADJUSTMENTS[1] = COLUMN;
    DIRECTION_ADJUSTMENTS[2] = -1;
    DIRECTION_ADJUSTMENTS[3] = 1;
    DIRECTION_ADJUSTMENTS[4] = -COLUMN - 1;
    DIRECTION_ADJUSTMENTS[5] = -COLUMN + 1;
    DIRECTION_ADJUSTMENTS[6] = COLUMN - 1;
    DIRECTION_ADJUSTMENTS[7] = COLUMN + 1;

    // setup the board here by finding the orb using the string
    for (int i = 0; i < board_size; i++) {
        char orb_char = board_string[i];
        // find the orb name from ORB_WEB_NAME and make it a number
        bool found = false;
        for (orb j = 0; j < ORB_COUNT; j++) {
            if (orb_char == ORB_WEB_NAME[j]) {
                found = true;
                BOARD[i] = j;
                ORB_COUNTER[j]++;
                break;
            }
        }

        if (!found) {
            printf("orb %c not found, only RBGLDHJP are valid\n", orb_char);
            exit(1);
        }
    }

    MAX_COMBO = calc_max_combo(ORB_COUNTER, BOARD_SIZE, MIN_ERASE);
}

void solver::set_min_erase(int min_erase) {
    // min 3, max 5 for now
    if (min_erase < 3) {
        min_erase = 3;
        DEBUG_PRINT("min_erase is too small, set to 3\n");
    } else if (min_erase > 5) {
        min_erase = 5;
        DEBUG_PRINT("min_erase is too large, set to 5\n");
    }
    MIN_ERASE = min_erase;
}

void solver::set_search_depth(int depth) {
    if (depth > MAX_DEPTH)
        depth = MAX_DEPTH;
    SEARCH_DEPTH = depth;
}

void solver::set_beam_size(int beam_size) {
    if (beam_size < MIN_BEAM_SIZE)
        beam_size = MIN_BEAM_SIZE;
    BEAM_SIZE = beam_size;
}

void solver::set_diagonal(bool diagonal) {
    ALLOW_DIAGONAL = diagonal;
}

void solver::set_profiles(profile* profiles, int count) {
    PROFILES = profiles;
    PROFILE_COUNT = count;
    for (int i = 0; i < count; i++) {
        // use the largest threshold
        if (STOP_THRESHOLD < profiles[i].stop_threshold)
            STOP_THRESHOLD = profiles[i].stop_threshold;
    }
}

void solver::print_board(const game_board& board) const {
    printf("Board: ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        auto orb = board[i];
        if (orb == 0)
            printf("P");
        else
            printf("%c", ORB_WEB_NAME[orb]);
    }
    printf("\n");
}

void solver::print_state(const state& state) const {
    printf("=============== STATE ===============\n");
    if (state.step == 0) {
        printf("Invalid state\n");
        exit(-1);
    }

    printf("Score: %d\n", state.score);
    printf("Combo: %d/%d\n", state.combo, MAX_COMBO);
    printf("Step: %d\n", state.step);
    print_board(state.board);
    print_route(state.route, state.step, state.begin);
    printf("Goal: %d\n", state.goal);
    printf("=====================================\n");
}

void solver::print_route(const route_list& route,
                         const int step,
                         const int begin) const {
    printf("Route: |%d| - ", begin);
    int max_index = step / ROUTE_PER_LIST;
    // in case, it doesn't fill up the space, check the offset
    int offset = step % ROUTE_PER_LIST;

    int count = 0;
    int index = 0;
    while (index <= max_index) {
        auto curr = route[index];
        int limit = ROUTE_PER_LIST;
        if (index == max_index) {
            limit = offset;
            // shift the number to the left
            curr <<= (ROUTE_PER_LIST - offset) * 3;
        }

        for (int i = 0; i < limit; i++) {
            // get first 3 bits and shift to the right, 3 * 20
            int dir = (curr & ROUTE_MASK) >> 60;
            // printf("%d", dir);
            printf("%c", DIRECTION_NAME[dir]);
            count++;
            // prepare for the next step
            curr <<= 3;
        }
        index++;
    }
    printf("\n");
    if (count != step) {
        printf("count (%d) should be equal to step (%d)\n", count, step);
        exit(1);
    }
}

// This is for debugging only, don't use it in pazusoba.cpp
std::string solver::get_board_string(const game_board& board) const {
    char board_string[MAX_BOARD_LENGTH + 1]{};
    for (int i = 0; i < MAX_BOARD_LENGTH; i++) {
        auto orb = board[i];
        if (orb == 0)
            break;
        board_string[i] = ORB_WEB_NAME[orb];
    }
    return std::string(board_string);
}

void solver::check_3x3_squares(game_board& board, combo_list& list, visit_board& visited_location) {
    DEBUG_PRINT("=== check_3x3_squares called ===\n");
    int squares_found = 0;
    
    // Check each possible top-left corner of a 3x3 square
    for (int row = 0; row <= ROW - 3; row++) {
        for (int col = 0; col <= COLUMN - 3; col++) {
            int top_left = INDEX_OF(row, col);
            auto orb = board[top_left];
            
            if (orb == 0) continue;  // already erased
            
            // Check if this 3x3 area forms a square of the same color
            bool is_square = true;
            std::unordered_set<int> square_locations;
            
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int index = INDEX_OF(row + i, col + j);
                    if (board[index] != orb) {
                        is_square = false;
                        break;
                    }
                    square_locations.insert(index);
                }
                if (!is_square) break;
            }
            
            if (is_square) {
                DEBUG_PRINT("3x3 square found at (%d,%d) with color %d\n", row, col, orb);
                squares_found++;
                
                // Create a combo for this 3x3 square
                combo c(orb);
                c.loc = square_locations;
                
                // Mark all positions as visited and erase them
                for (int loc : square_locations) {
                    visited_location[loc] = true;
                    board[loc] = 0;
                }
                
                list.push_back(c);
            }
        }
    }
    
    DEBUG_PRINT("3x3 squares found: %d\n", squares_found);
}

bool solver::is_3x3_square(const std::unordered_set<int>& locations, int column) const {
    if (locations.size() != 9) return false;
    
    // Find the minimum row and column to determine top-left corner
    int min_row = ROW, min_col = COLUMN;
    for (int loc : locations) {
        int row = loc / column;
        int col = loc % column;
        min_row = std::min(min_row, row);
        min_col = std::min(min_col, col);
    }
    
    // Check if all 9 positions in the 3x3 square starting from (min_row, min_col) are present
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int expected_loc = INDEX_OF(min_row + i, min_col + j);
            if (locations.find(expected_loc) == locations.end()) {
                return false;
            }
        }
    }
    
    return true;
}

void solver::usage() const {
    printf(
        "\nusage: pazusoba [board string] [min erase] [max steps] [max "
        "beam size] [diagonal]\nboard string\t-- "
        "eg. RHLBDGPRHDRJPJRHHJGRDRHLGLPHBB\nmin erase\t-- 3 to 5\nmax "
        "steps\t-- maximum steps before the program stops "
        "searching\nmax beam size\t-- the width of the search space, "
        "larger number means slower speed but better results\ndiagonal\t-- "
        "--diagonal or -d to enable diagonal movement (default: disabled)\n\nMore "
        "at https://github.com/pazusoba/core\n\n");
    exit(0);
}
}  // namespace pazusoba
