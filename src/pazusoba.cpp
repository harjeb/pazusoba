// pazusoba.cpp
// Try to improve the performance of the solver while being flexible enough

// Compile with
// mac: clang++ -std=c++11 -fopenmp -O2 pazusoba.cpp -o pazusoba
// windows: g++ -std=c++11 -fopenmp -O2 pazusoba.cpp -o pazusoba

#include <pazusoba/core.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <thread>
#include <vector>

namespace pazusoba {
state solver::adventure() {
    int REAL_BEAM_SIZE = BEAM_SIZE * 1.4;
    int max_children = ALLOW_DIAGONAL ? DIRECTION_COUNT : 4;
    VISITED.clear();
    // setup the state, non blocking
    std::vector<state> look;
    look.reserve(REAL_BEAM_SIZE);
    // insert to temp, sort and copy back to look
    std::vector<state> temp;
    temp.resize(REAL_BEAM_SIZE * max_children);
    // TODO: using array can definitely things a lot because the vector needs to
    // write a lot of useless data before using it, reverse is better but the
    // address calculation can be tricky

    state best_state;
    bool found_max_combo = false;

    // assign all possible states to look
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (BLOCKED[i])
            continue;
        state new_state;
        new_state.curr = i;
        new_state.prev = i;
        new_state.begin = i;
        new_state.score = MIN_STATE_SCORE + 1;
        look.push_back(new_state);
    }

    // setup threading
    int processor_count = std::thread::hardware_concurrency();
    if (processor_count <= 0)
        processor_count = 1;
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
        for (auto& s : temp)
            s.score = MIN_STATE_SCORE;

        // #pragma omp parallel for
        for (int thread_num = 0; thread_num < processor_count; thread_num++) {
            threads.emplace_back([&, thread_num, look_size_thread] {
                int start_index = thread_num * (look_size_thread);
                int end_index = (thread_num == processor_count - 1)
                                    ? look_size
                                    : start_index + look_size_thread;
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
        auto top_end = begin + std::min<int>(REAL_BEAM_SIZE, temp.size());
        auto end = temp.end();
        std::nth_element(begin, top_end, end, std::greater<state>());
        std::sort(begin, top_end, std::greater<state>());

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
            if (VISITED.find(curr.hash) != VISITED.end()) {
                index--;
            } else {
                VISITED.insert(curr.hash);
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
    int max_children = ALLOW_DIAGONAL ? DIRECTION_COUNT : 4;
    int slot = 0;
    for (int i = 0; i < count; i++) {
        int curr_row = curr / COLUMN;
        int curr_col = curr % COLUMN;
        int next_row = curr_row;
        int next_col = curr_col;
        switch (i) {
            case up:
                next_row--;
                break;
            case down:
                next_row++;
                break;
            case left:
                next_col--;
                break;
            case right:
                next_col++;
                break;
            case up_left:
                next_row--;
                next_col--;
                break;
            case up_right:
                next_row--;
                next_col++;
                break;
            case down_left:
                next_row++;
                next_col--;
                break;
            case down_right:
                next_row++;
                next_col++;
                break;
            default:
                continue;
        }
        if (next_row < 0 || next_row >= ROW || next_col < 0 || next_col >= COLUMN)
            continue;  // invalid, out of bound
        int next = INDEX_OF(next_row, next_col);
        if (next == prev)
            continue;  // invalid, same position
        if (BLOCKED[next])
            continue;  // blocked cell cannot be entered

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
        new_state.hash = hash::pazusoba_hash(new_board.data(), new_state.curr);

        // backup the board
        evaluate(new_board, new_state);

        // insert to the states using compact per-node slots
        states[loc * max_children + slot] = new_state;
        slot++;
    }
}

void solver::evaluate(game_board& board, state& new_state) {
    int score = 0;
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

    auto adjacency_score = [&](const profile* p) {
        int guide = 0;
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (board[i] == 0)
                continue;
            int weight = 4;
            if (p != nullptr && p->orbs[board[i]])
                weight = 8;
            if (i % COLUMN != COLUMN - 1 && board[i] == board[i + 1])
                guide += weight;
            if (i + COLUMN < BOARD_SIZE && board[i] == board[i + COLUMN])
                guide += weight;
        }
        return std::min(guide, 200);
    };

    // erase the board and find out the combo number
    combo_list list;  // TODO: 515ms here, destructor is slow
    combo_list all_list;

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
            combo += combo_count;
            all_list.insert(all_list.end(), list.begin(), list.end());
            DEBUG_PRINT("Current combo count: %d\n", combo);

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
                int preferred_combo = 0;
                for (const auto& c : all_list) {
                    if (profile.orbs[c.info])
                        preferred_combo++;
                }
                int guide = adjacency_score(&profile);
                if (target == -1) {
                    // max combo
                    score += combo * 1000 + guide - new_state.step;
                    if (profile.colour_target <= 0) {
                        score += preferred_combo * 300;
                    } else {
                        int lack = profile.colour_target - preferred_combo;
                        if (lack > 0)
                            score -= lack * 1500;
                        else
                            score += preferred_combo * 300;
                    }
                    if (combo == MAX_COMBO &&
                        (profile.colour_target <= 0 || preferred_combo >= profile.colour_target))
                        goal++;
                } else {
                    // only do max target combo
                    if (combo < target)
                        score -= (target - combo) * 1000;
                    if (combo == target)
                        score += combo * 1000 + guide - new_state.step;
                    else if (target > 7)
                        score -= 500;

                    if (profile.colour_target <= 0) {
                        score += preferred_combo * 300;
                    } else {
                        int lack = profile.colour_target - preferred_combo;
                        if (lack > 0)
                            score -= lack * 1500;
                        else
                            score += preferred_combo * 300;
                    }

                    if (combo == target &&
                        (profile.colour_target <= 0 || preferred_combo >= profile.colour_target))
                        goal++;
                }
            } break;

            case colour: {
                int colour_counter[ORB_COUNT]{0};
                for (const auto& c : all_list) {
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
                for (const auto& c : all_list) {
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
                bool has_orb_filter = false;
                for (int j = 0; j < ORB_COUNT; ++j) {
                    if (profile.orbs[j]) {
                        has_orb_filter = true;
                        break;
                    }
                }

                for (const auto& c : all_list) {
                    if (has_orb_filter && !profile.orbs[c.info])
                        continue;
                    int connected_count = c.loc.size();
                    if (ORB_COUNTER[c.info] >= target) {
                        if (connected_count < target) {
                            score += (connected_count - MIN_ERASE) * 10;
                        } else if (connected_count == target) {
                            fulfilled = true;
                            score += 100;
                        } else {
                            score -= (connected_count - target) * 50;
                        }
                    }
                }

                score += combo * 20;
                if (fulfilled)
                    score += 20000;

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
                for (const auto& c : all_list) {
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
                for (const auto& c : all_list) {
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
                for (const auto& c : all_list) {
                    if (profile.orbs[c.info] && ORB_COUNTER[c.info] >= 9) {
                        int size = c.loc.size();
                        if (size >= 9) {
                            // Check if it forms a 3x3 square
                            if (is_3x3_square(c.loc, COLUMN)) {
                                score += 50000;  // MASSIVE score for 3x3
                                goal++;
                                found_3x3 = true;
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
                for (const auto& c : all_list) {
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
    visit_board marked{0};
    visit_board visited{0};

    for (int row = 0; row < ROW; ++row) {
        int col = 0;
        while (col < COLUMN) {
            int start = col;
            orb current = board[INDEX_OF(row, col)];
            while (col < COLUMN && board[INDEX_OF(row, col)] == current)
                ++col;

            int length = col - start;
            if (current != 0 && length >= MIN_ERASE) {
                for (int c = start; c < col; ++c)
                    marked[INDEX_OF(row, c)] = true;
            }
        }
    }

    for (int col = 0; col < COLUMN; ++col) {
        int row = 0;
        while (row < ROW) {
            int start = row;
            orb current = board[INDEX_OF(row, col)];
            while (row < ROW && board[INDEX_OF(row, col)] == current)
                ++row;

            int length = row - start;
            if (current != 0 && length >= MIN_ERASE) {
                for (int r = start; r < row; ++r)
                    marked[INDEX_OF(r, col)] = true;
            }
        }
    }

    for (int start = 0; start < BOARD_SIZE; ++start) {
        if (!marked[start] || visited[start])
            continue;

        orb current = board[start];
        combo c(current);
        std::queue<int> q;
        q.push(start);
        visited[start] = true;

        while (!q.empty()) {
            int loc = q.front();
            q.pop();
            c.loc.insert(loc);

            for (int i = 0; i < 4; ++i) {
                int next = loc + DIRECTION_ADJUSTMENTS[i];
                if (i == up && loc < COLUMN)
                    continue;
                if (i == down && next >= BOARD_SIZE)
                    continue;
                if (i == left && loc % COLUMN == 0)
                    continue;
                if (i == right && loc % COLUMN == COLUMN - 1)
                    continue;
                if (next < 0 || next >= BOARD_SIZE)
                    continue;
                if (!marked[next] || visited[next] || board[next] != current)
                    continue;

                visited[next] = true;
                q.push(next);
            }
        }

        if ((int)c.loc.size() >= MIN_ERASE) {
            for (int loc : c.loc)
                board[loc] = 0;
            list.push_back(c);
            DEBUG_PRINT("Combo added: size %d, color %d\n", (int)c.loc.size(), c.info);
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

    for (int i = 5; i < argc; ++i) {
        if (strcmp(argv[i], "--diagonal") == 0 || strcmp(argv[i], "-d") == 0) {
            set_diagonal(true);
        } else if (strncmp(argv[i], "--blocked=", 10) == 0) {
            std::vector<int> positions;
            std::stringstream ss(argv[i] + 10);
            std::string token;
            while (std::getline(ss, token, ',')) {
                if (!token.empty())
                    positions.push_back(std::atoi(token.c_str()));
            }
            set_blocked(positions.data(), (int)positions.size());
        } else if (strncmp(argv[i], "--blocked-rc=", 13) == 0) {
            std::vector<int> positions;
            std::stringstream ss(argv[i] + 13);
            std::string token;
            while (std::getline(ss, token, ',')) {
                size_t sep = token.find(':');
                if (sep == std::string::npos)
                    continue;
                int row = std::atoi(token.substr(0, sep).c_str());
                int col = std::atoi(token.substr(sep + 1).c_str());
                positions.push_back(INDEX_OF(row, col));
            }
            set_blocked(positions.data(), (int)positions.size());
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
    BOARD.fill(0);
    ORB_COUNTER.fill(0);
    VISITED.clear();
    BLOCKED.fill(false);
    BLOCKED_COUNT = 0;

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

void solver::set_blocked(const int* positions, int count) {
    BLOCKED.fill(false);
    BLOCKED_COUNT = 0;
    for (int i = 0; i < count; ++i) {
        int p = positions[i];
        if (p < 0 || p >= BOARD_SIZE) {
            printf("WARNING: blocked position %d ignored, out of board\n", p);
            continue;
        }
        if (!BLOCKED[p]) {
            BLOCKED[p] = true;
            BLOCKED_COUNT++;
        }
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
