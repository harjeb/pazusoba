#include <pazusoba/shape.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace pazusoba {
namespace {

const int INF = 1 << 28;
const std::array<int, 8> DR{{-1, 1, 0, 0, -1, -1, 1, 1}};
const std::array<int, 8> DC{{0, 0, -1, 1, -1, 1, -1, 1}};
const std::array<char, 8> MOVE_NAME{{'U', 'D', 'L', 'R', 'Q', 'E', 'Z', 'C'}};

struct Candidate {
    int cost = INF;
    int color = 0;
    std::vector<int> cells;
};

int pos_of(int row, int col, int cols) {
    return row * cols + col;
}

int row_of(int pos, int cols) {
    return pos / cols;
}

int col_of(int pos, int cols) {
    return pos % cols;
}

bool inside(int row, int col, int rows, int cols) {
    return row >= 0 && row < rows && col >= 0 && col < cols;
}

std::vector<int> neighbors(int pos, int rows, int cols, bool allow_diagonal) {
    std::vector<int> out;
    int row = row_of(pos, cols);
    int col = col_of(pos, cols);
    int direction_count = allow_diagonal ? 8 : 4;
    for (int i = 0; i < direction_count; ++i) {
        int nr = row + DR[i];
        int nc = col + DC[i];
        if (inside(nr, nc, rows, cols))
            out.push_back(pos_of(nr, nc, cols));
    }
    return out;
}

char move_char(int from, int to, int cols) {
    int dr = row_of(to, cols) - row_of(from, cols);
    int dc = col_of(to, cols) - col_of(from, cols);
    for (int i = 0; i < 8; ++i) {
        if (DR[i] == dr && DC[i] == dc)
            return MOVE_NAME[i];
    }
    return '?';
}

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

bool has_color_filter(const shape_request& request) {
    for (int i = 0; i < ORB_COUNT; ++i) {
        if (request.colors[i])
            return true;
    }
    return false;
}

bool color_allowed(const shape_request& request, int color, bool prefer_pass) {
    if (request.mode == color_auto)
        return true;
    if (!has_color_filter(request))
        return true;
    if (request.mode == color_only || prefer_pass)
        return request.colors[color];
    return true;
}

int assign_cost_for_cells(const std::string& board,
                          int rows,
                          int cols,
                          int color,
                          const std::vector<int>& cells,
                          const std::array<bool, MAX_BOARD_LENGTH>& blocked) {
    (void)rows;
    std::vector<int> targets;
    std::vector<int> orbs;
    std::vector<bool> target_mask(board.size(), false);
    for (int cell : cells) {
        target_mask[cell] = true;
        if (blocked[cell]) {
            if (board[cell] != ORB_WEB_NAME[color])
                return INF;
            continue;
        }
        targets.push_back(cell);
    }
    for (int i = 0; i < (int)board.size(); ++i) {
        if (blocked[i] || board[i] != ORB_WEB_NAME[color])
            continue;
        orbs.push_back(i);
    }
    if (orbs.size() < targets.size())
        return INF;
    if (targets.empty())
        return 0;

    int full = 1 << targets.size();
    std::vector<int> dp(full, INF);
    dp[0] = 0;
    for (int orb_pos : orbs) {
        std::vector<int> next = dp;
        for (int mask = 0; mask < full; ++mask) {
            if (dp[mask] >= INF)
                continue;
            for (int j = 0; j < (int)targets.size(); ++j) {
                if (mask & (1 << j))
                    continue;
                int dist = std::abs(row_of(orb_pos, cols) - row_of(targets[j], cols)) +
                           std::abs(col_of(orb_pos, cols) - col_of(targets[j], cols));
                int nmask = mask | (1 << j);
                next[nmask] = std::min(next[nmask], dp[mask] + dist);
            }
        }
        dp.swap(next);
    }
    return dp[full - 1];
}

bool isolated_enough(const std::string& board,
                     int rows,
                     int cols,
                     int color,
                     const std::vector<int>& cells) {
    std::vector<bool> in_shape(board.size(), false);
    for (int cell : cells)
        in_shape[cell] = true;
    for (int cell : cells) {
        for (int next : neighbors(cell, rows, cols, false)) {
            if (!in_shape[next] && board[next] == ORB_WEB_NAME[color])
                return false;
        }
    }
    return true;
}

std::vector<Candidate> ranked_candidates(const std::string& board,
                                         int rows,
                                         int cols,
                                         const shape_request& request,
                                         const std::array<bool, MAX_BOARD_LENGTH>& blocked,
                                         bool prefer_pass) {
    shape_template templ = make_shape_template(request.shape);
    std::array<int, ORB_COUNT> counts{};
    for (char ch : board) {
        for (int color = 1; color < ORB_COUNT; ++color) {
            if (ch == ORB_WEB_NAME[color]) {
                counts[color]++;
                break;
            }
        }
    }

    std::vector<Candidate> out;
    for (int color = 1; color < ORB_COUNT; ++color) {
        if (!color_allowed(request, color, prefer_pass))
            continue;
        if (counts[color] < templ.orb_count(rows, cols))
            continue;
        for (const auto& cells : templ.placements(rows, cols)) {
            int cost = assign_cost_for_cells(board, rows, cols, color, cells, blocked);
            if (cost >= INF)
                continue;
            if (request.strict_isolation && cost == 0 &&
                !isolated_enough(board, rows, cols, color, cells)) {
                continue;
            }
            Candidate candidate;
            candidate.cost = cost;
            candidate.color = color;
            candidate.cells = cells;
            out.push_back(candidate);
        }
    }
    std::sort(out.begin(), out.end(), [](const Candidate& a, const Candidate& b) {
        if (a.cost != b.cost)
            return a.cost < b.cost;
        return a.color < b.color;
    });
    return out;
}

int pick_nearest_free_orb(const std::string& board,
                          int rows,
                          int cols,
                          int color,
                          int goal,
                          const std::vector<bool>& locked,
                          int finger) {
    (void)rows;
    int best = -1;
    int best_dist = INF;
    for (int pos = 0; pos < (int)board.size(); ++pos) {
        if (locked[pos] || pos == finger || board[pos] != ORB_WEB_NAME[color])
            continue;
        int dist = std::abs(row_of(pos, cols) - row_of(goal, cols)) +
                   std::abs(col_of(pos, cols) - col_of(goal, cols));
        if (dist < best_dist) {
            best_dist = dist;
            best = pos;
        }
    }
    return best;
}

bool move_orb_bfs(std::string& board,
                  int rows,
                  int cols,
                  int orb_from,
                  int goal,
                  int& finger,
                  const std::vector<bool>& locked,
                  bool allow_diagonal,
                  std::string& route) {
    if (orb_from == goal)
        return true;

    const int size = rows * cols;
    struct Node {
        int orb;
        int finger;
    };

    std::vector<int> parent(size * size, -1);
    std::vector<char> pmove(size * size, '?');
    auto id_of = [size](int orb, int finger_pos) {
        return orb * size + finger_pos;
    };

    std::queue<Node> q;
    int start_id = id_of(orb_from, finger);
    parent[start_id] = start_id;
    q.push({orb_from, finger});
    int found_id = -1;

    while (!q.empty()) {
        Node node = q.front();
        q.pop();
        if (node.orb == goal) {
            found_id = id_of(node.orb, node.finger);
            break;
        }
        for (int next : neighbors(node.finger, rows, cols, allow_diagonal)) {
            if (locked[next])
                continue;
            int next_orb = (next == node.orb) ? node.finger : node.orb;
            int nid = id_of(next_orb, next);
            if (parent[nid] != -1)
                continue;
            parent[nid] = id_of(node.orb, node.finger);
            pmove[nid] = move_char(node.finger, next, cols);
            q.push({next_orb, next});
        }
    }

    if (found_id < 0)
        return false;

    std::string seq;
    for (int id = found_id; id != start_id; id = parent[id])
        seq.push_back(pmove[id]);
    std::reverse(seq.begin(), seq.end());

    for (char move : seq) {
        int next = apply_move(finger, move, cols);
        std::swap(board[finger], board[next]);
        route.push_back(move);
        finger = next;
    }
    return true;
}

bool move_two_orbs_bfs(std::string& board,
                       int rows,
                       int cols,
                       int color,
                       int goal1,
                       int goal2,
                       int& finger,
                       const std::vector<bool>& locked,
                       bool allow_diagonal,
                       std::string& route) {
    if (board[goal1] == ORB_WEB_NAME[color] && board[goal2] == ORB_WEB_NAME[color])
        return true;

    const int size = rows * cols;
    std::vector<int> orbs;
    for (int pos = 0; pos < size; ++pos) {
        if (locked[pos] || pos == finger || board[pos] != ORB_WEB_NAME[color])
            continue;
        orbs.push_back(pos);
    }
    if (orbs.size() < 2)
        return false;

    auto encode = [size](int orb1, int orb2, int finger_pos) {
        return (orb1 * size + orb2) * size + finger_pos;
    };

    for (size_t a = 0; a < orbs.size(); ++a) {
        for (size_t b = a + 1; b < orbs.size(); ++b) {
            int start_id = encode(orbs[a], orbs[b], finger);
            std::unordered_map<int, int> parent;
            std::unordered_map<int, char> pmove;
            std::queue<int> q;
            parent[start_id] = start_id;
            q.push(start_id);
            int found = -1;

            while (!q.empty()) {
                int id = q.front();
                q.pop();
                int curr_finger = id % size;
                int tmp = id / size;
                int orb2 = tmp % size;
                int orb1 = tmp / size;

                bool done = (orb1 == goal1 && orb2 == goal2) ||
                            (orb1 == goal2 && orb2 == goal1);
                if (done) {
                    found = id;
                    break;
                }

                for (int next : neighbors(curr_finger, rows, cols, allow_diagonal)) {
                    if (locked[next])
                        continue;
                    int next_orb1 = (next == orb1) ? curr_finger : orb1;
                    int next_orb2 = (next == orb2) ? curr_finger : orb2;
                    if (next_orb1 == next_orb2)
                        continue;
                    int nid = encode(next_orb1, next_orb2, next);
                    if (parent.find(nid) != parent.end())
                        continue;
                    parent[nid] = id;
                    pmove[nid] = move_char(curr_finger, next, cols);
                    q.push(nid);
                }
            }

            if (found < 0)
                continue;

            std::string seq;
            for (int id = found; id != start_id; id = parent[id])
                seq.push_back(pmove[id]);
            std::reverse(seq.begin(), seq.end());

            for (char move : seq) {
                int next = apply_move(finger, move, cols);
                std::swap(board[finger], board[next]);
                route.push_back(move);
                finger = next;
            }
            return board[goal1] == ORB_WEB_NAME[color] &&
                   board[goal2] == ORB_WEB_NAME[color];
        }
    }
    return false;
}

std::vector<std::vector<int>> make_orders(int count) {
    std::vector<int> forward;
    for (int i = 0; i < count; ++i)
        forward.push_back(i);
    std::vector<int> reverse = forward;
    std::reverse(reverse.begin(), reverse.end());
    std::vector<std::vector<int>> orders;
    orders.push_back(forward);
    orders.push_back(reverse);
    if (count == 9) {
        orders.push_back({0, 3, 6, 1, 4, 7, 2, 5, 8});
        orders.push_back({2, 1, 0, 5, 4, 3, 8, 7, 6});
        orders.push_back({6, 3, 0, 7, 4, 1, 8, 5, 2});
    }
    return orders;
}

bool run_constructive_for_candidate(std::string& board,
                                    int rows,
                                    int cols,
                                    const Candidate& target,
                                    const std::array<bool, MAX_BOARD_LENGTH>& blocked,
                                    bool allow_diagonal,
                                    int& start_finger,
                                    std::string& route,
                                    std::string& note) {
    const int size = rows * cols;
    std::vector<bool> in_shape(size, false);
    for (int cell : target.cells)
        in_shape[cell] = true;

    int initial_finger = -1;
    for (int i = 0; i < size; ++i) {
        if (!blocked[i] && !in_shape[i] && board[i] != ORB_WEB_NAME[target.color]) {
            initial_finger = i;
            break;
        }
    }
    if (initial_finger < 0) {
        for (int i = 0; i < size; ++i) {
            if (!blocked[i] && !in_shape[i]) {
                initial_finger = i;
                break;
            }
        }
    }
    if (initial_finger < 0) {
        note = "no movable start cell";
        return false;
    }

    for (const auto& order : make_orders((int)target.cells.size())) {
        std::string attempt = board;
        std::string attempt_route;
        int finger = initial_finger;
        std::vector<bool> locked(size, false);
        for (int i = 0; i < size; ++i) {
            if (blocked[i])
                locked[i] = true;
        }

        bool ok = true;
        for (int oi = 0; oi < (int)order.size(); ++oi) {
            int remaining_missing = 0;
            for (int ri = oi; ri < (int)order.size(); ++ri) {
                int remaining_goal = target.cells[order[ri]];
                if (attempt[remaining_goal] != ORB_WEB_NAME[target.color])
                    remaining_missing++;
            }
            if (remaining_missing <= 2)
                break;

            int goal = target.cells[order[oi]];
            if (attempt[goal] == ORB_WEB_NAME[target.color]) {
                locked[goal] = true;
                continue;
            }
            int orb = pick_nearest_free_orb(attempt, rows, cols, target.color, goal, locked, finger);
            if (orb < 0) {
                note = "no movable target orb";
                ok = false;
                break;
            }
            if (!move_orb_bfs(attempt, rows, cols, orb, goal, finger, locked, allow_diagonal,
                              attempt_route)) {
                note = "single-orb BFS blocked";
                ok = false;
                break;
            }
            if (attempt[goal] != ORB_WEB_NAME[target.color]) {
                note = "single-orb replay failed";
                ok = false;
                break;
            }
            locked[goal] = true;
        }
        if (!ok)
            continue;

        std::vector<int> missing_goals;
        for (int index : order) {
            int goal = target.cells[index];
            if (attempt[goal] != ORB_WEB_NAME[target.color])
                missing_goals.push_back(goal);
        }

        if (missing_goals.size() == 1) {
            int goal = missing_goals[0];
            int orb = pick_nearest_free_orb(attempt, rows, cols, target.color, goal, locked, finger);
            if (orb < 0 ||
                !move_orb_bfs(attempt, rows, cols, orb, goal, finger, locked, allow_diagonal,
                              attempt_route)) {
                note = "final single-orb BFS blocked";
                continue;
            }
        } else if (missing_goals.size() >= 2) {
            int goal1 = missing_goals[missing_goals.size() - 2];
            int goal2 = missing_goals[missing_goals.size() - 1];
            if (!move_two_orbs_bfs(attempt, rows, cols, target.color, goal1, goal2, finger,
                                   locked, allow_diagonal, attempt_route)) {
                note = "two-orb BFS blocked";
                continue;
            }
        }

        bool formed = true;
        for (int cell : target.cells) {
            if (attempt[cell] != ORB_WEB_NAME[target.color]) {
                formed = false;
                break;
            }
        }
        if (formed) {
            board = attempt;
            start_finger = initial_finger;
            route = attempt_route;
            return true;
        }
        note = "final board does not contain target shape";
    }

    return false;
}

std::string candidate_note(const Candidate& candidate) {
    std::ostringstream ss;
    ss << "constructive color=" << ORB_WEB_NAME[candidate.color]
       << " cost=" << candidate.cost;
    return ss.str();
}

std::pair<int, int> evaluate_combo(const std::string& board) {
    solver s;
    s.set_board(board.c_str());
    auto copy = s.board();
    state evaluated;
    s.evaluate(copy, evaluated);
    return std::make_pair((int)evaluated.combo, s.max_combo());
}

}  // namespace

std::vector<std::vector<int>> shape_template::placements(int rows, int cols) const {
    std::vector<std::vector<int>> out;
    if (kind == shape_3x3_square) {
        for (int row = 0; row <= rows - 3; ++row) {
            for (int col = 0; col <= cols - 3; ++col) {
                std::vector<int> cells;
                for (int dr = 0; dr < 3; ++dr) {
                    for (int dc = 0; dc < 3; ++dc)
                        cells.push_back(pos_of(row + dr, col + dc, cols));
                }
                out.push_back(cells);
            }
        }
    } else if (kind == shape_cross) {
        for (int row = 1; row < rows - 1; ++row) {
            for (int col = 1; col < cols - 1; ++col) {
                int center = pos_of(row, col, cols);
                out.push_back({center, center - 1, center + 1, center - cols, center + cols});
            }
        }
    } else if (kind == shape_full_row) {
        for (int row = 0; row < rows; ++row) {
            std::vector<int> cells;
            for (int col = 0; col < cols; ++col)
                cells.push_back(pos_of(row, col, cols));
            out.push_back(cells);
        }
    } else if (kind == shape_full_column) {
        for (int col = 0; col < cols; ++col) {
            std::vector<int> cells;
            for (int row = 0; row < rows; ++row)
                cells.push_back(pos_of(row, col, cols));
            out.push_back(cells);
        }
    }
    return out;
}

int shape_template::orb_count(int rows, int cols) const {
    if (kind == shape_3x3_square)
        return 9;
    if (kind == shape_cross)
        return 5;
    if (kind == shape_full_row)
        return cols;
    if (kind == shape_full_column)
        return rows;
    return 0;
}

shape_template make_shape_template(int kind) {
    shape_template templ;
    templ.kind = kind;
    return templ;
}

bool board_has_shape(const std::string& board,
                     int rows,
                     int cols,
                     int kind,
                     char* color) {
    shape_template templ = make_shape_template(kind);
    for (const auto& cells : templ.placements(rows, cols)) {
        if (cells.empty())
            continue;
        char target = board[cells[0]];
        if (target == ORB_WEB_NAME[0])
            continue;
        bool ok = true;
        for (int cell : cells) {
            if (board[cell] != target) {
                ok = false;
                break;
            }
        }
        if (ok) {
            if (color)
                *color = target;
            return true;
        }
    }
    return false;
}

shape_result solve_shape(const std::string& board,
                         int rows,
                         int cols,
                         const shape_request& request) {
    std::array<bool, MAX_BOARD_LENGTH> blocked{};
    blocked.fill(false);
    return solve_shape(board, rows, cols, request, blocked);
}

shape_result solve_shape(const std::string& board,
                         int rows,
                         int cols,
                         const shape_request& request,
                         const std::array<bool, MAX_BOARD_LENGTH>& blocked) {
    shape_result result;
    result.final_board = board;
    if ((int)board.size() != rows * cols || board.size() > MAX_BOARD_LENGTH) {
        result.note = "invalid board size";
        return result;
    }

    std::vector<Candidate> candidates;
    if (request.mode == color_prefer && has_color_filter(request)) {
        candidates = ranked_candidates(board, rows, cols, request, blocked, true);
        if (candidates.empty())
            candidates = ranked_candidates(board, rows, cols, request, blocked, false);
    } else {
        candidates = ranked_candidates(board, rows, cols, request, blocked, false);
    }

    if (candidates.empty()) {
        result.note = "no feasible candidate";
        return result;
    }

    shape_result best;
    best.final_board = board;
    std::string last_note = "no candidate tried";
    int limit = (int)candidates.size();
    for (int i = 0; i < limit; ++i) {
        std::string attempt = board;
        std::string route;
        int start = -1;
        if (!run_constructive_for_candidate(attempt, rows, cols, candidates[i], blocked,
                                            request.allow_diagonal, start, route, last_note)) {
            continue;
        }
        auto combo = evaluate_combo(attempt);
        shape_result current;
        current.success = true;
        current.steps = (int)route.size();
        current.start = start;
        current.color = candidates[i].color;
        current.combo = combo.first;
        current.max_combo = combo.second;
        current.route = route;
        current.final_board = attempt;
        current.note = candidate_note(candidates[i]);

        if (!best.success || current.combo > best.combo ||
            (current.combo == best.combo && current.steps < best.steps)) {
            best = current;
        }
    }

    if (best.success) {
        best.note += " combo-optimized";
        return best;
    }

    result.note = last_note;
    return result;
}

}  // namespace pazusoba
