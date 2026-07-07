#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

constexpr int ROWS = 5;
constexpr int COLS = 6;
constexpr int SIZE = ROWS * COLS;
constexpr int INF = 1 << 28;
const std::array<int, 4> DR{{-1, 1, 0, 0}};
const std::array<int, 4> DC{{0, 0, -1, 1}};
const std::array<char, 4> MOVE_NAME{{'U', 'D', 'L', 'R'}};

struct SearchResult {
    bool success = false;
    int steps = -1;
    int start = -1;
    double ms = 0.0;
    std::string route;
    std::string final_board;
    std::string note;
};

struct Candidate {
    int cost = INF;
    char color = '?';
    int top = 0;
    int left = 0;
};

struct HeuristicContext {
    Candidate target;
    std::vector<int> cells;
    int dist_to[9][SIZE]{};
};

struct PathState {
    std::string board;
    int curr = 0;
    int prev = -1;
    int steps = 0;
    int h = INF;
    int fixed_cost = INF;
    int start = -1;
    std::string route;
};

int row_of(int pos) {
    return pos / COLS;
}

int col_of(int pos) {
    return pos % COLS;
}

int pos_of(int row, int col) {
    return row * COLS + col;
}

bool inside(int row, int col) {
    return row >= 0 && row < ROWS && col >= 0 && col < COLS;
}

std::vector<int> neighbors(int pos) {
    std::vector<int> out;
    int r = row_of(pos);
    int c = col_of(pos);
    for (int i = 0; i < 4; ++i) {
        int nr = r + DR[i];
        int nc = c + DC[i];
        if (inside(nr, nc)) out.push_back(pos_of(nr, nc));
    }
    return out;
}

char move_char(int from, int to) {
    int dr = row_of(to) - row_of(from);
    int dc = col_of(to) - col_of(from);
    for (int i = 0; i < 4; ++i) {
        if (DR[i] == dr && DC[i] == dc) return MOVE_NAME[i];
    }
    return '?';
}

int apply_move(int pos, char mv) {
    switch (mv) {
        case 'U':
            return pos - COLS;
        case 'D':
            return pos + COLS;
        case 'L':
            return pos - 1;
        case 'R':
            return pos + 1;
        default:
            return pos;
    }
}

bool has_3x3(const std::string& board) {
    for (int top = 0; top <= ROWS - 3; ++top) {
        for (int left = 0; left <= COLS - 3; ++left) {
            char color = board[pos_of(top, left)];
            if (color == ' ') continue;
            bool ok = true;
            for (int r = 0; r < 3 && ok; ++r) {
                for (int c = 0; c < 3; ++c) {
                    if (board[pos_of(top + r, left + c)] != color) {
                        ok = false;
                        break;
                    }
                }
            }
            if (ok) return true;
        }
    }
    return false;
}

bool has_initial_line3(const std::string& board) {
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c <= COLS - 3; ++c) {
            char color = board[pos_of(r, c)];
            if (color == board[pos_of(r, c + 1)] && color == board[pos_of(r, c + 2)]) {
                return true;
            }
        }
    }
    for (int r = 0; r <= ROWS - 3; ++r) {
        for (int c = 0; c < COLS; ++c) {
            char color = board[pos_of(r, c)];
            if (color == board[pos_of(r + 1, c)] && color == board[pos_of(r + 2, c)]) {
                return true;
            }
        }
    }
    return false;
}

bool replay_route(const std::string& input, const SearchResult& result, std::string* error = nullptr) {
    if (!result.success) return true;
    if (result.start < 0 || result.start >= SIZE) {
        if (error) *error = "invalid start";
        return false;
    }

    std::string board = input;
    int curr = result.start;
    for (char mv : result.route) {
        int next = apply_move(curr, mv);
        if (next < 0 || next >= SIZE) {
            if (error) *error = "move out of board";
            return false;
        }
        if (std::abs(row_of(curr) - row_of(next)) + std::abs(col_of(curr) - col_of(next)) != 1) {
            if (error) *error = "move is not adjacent";
            return false;
        }
        std::swap(board[curr], board[next]);
        curr = next;
    }

    if (result.steps != static_cast<int>(result.route.size())) {
        if (error) *error = "step count does not match route length";
        return false;
    }
    if (board != result.final_board) {
        if (error) *error = "replayed final board mismatch";
        return false;
    }
    if (!has_3x3(board)) {
        if (error) *error = "replayed board has no 3x3";
        return false;
    }
    return true;
}

int assign_cost_for_window(const std::string& board, char color, int top, int left) {
    std::vector<int> targets;
    std::vector<int> orbs;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) targets.push_back(pos_of(top + r, left + c));
    }
    for (int i = 0; i < SIZE; ++i) {
        if (board[i] == color) orbs.push_back(i);
    }
    if (orbs.size() < 9) return INF;

    std::array<int, 1 << 9> dp;
    dp.fill(INF);
    dp[0] = 0;
    for (int orb : orbs) {
        auto next = dp;
        for (int mask = 0; mask < (1 << 9); ++mask) {
            if (dp[mask] >= INF) continue;
            for (int j = 0; j < 9; ++j) {
                if (mask & (1 << j)) continue;
                int dist = std::abs(row_of(orb) - row_of(targets[j])) +
                           std::abs(col_of(orb) - col_of(targets[j]));
                int nmask = mask | (1 << j);
                next[nmask] = std::min(next[nmask], dp[mask] + dist);
            }
        }
        dp = next;
    }
    return dp[(1 << 9) - 1];
}

Candidate best_candidate(const std::string& board) {
    std::array<int, 256> counts{};
    for (char ch : board) counts[static_cast<unsigned char>(ch)]++;

    Candidate best;
    for (int ch = 0; ch < 256; ++ch) {
        if (counts[ch] < 9) continue;
        char color = static_cast<char>(ch);
        for (int top = 0; top <= ROWS - 3; ++top) {
            for (int left = 0; left <= COLS - 3; ++left) {
                int cost = assign_cost_for_window(board, color, top, left);
                if (cost < best.cost) {
                    best.cost = cost;
                    best.color = color;
                    best.top = top;
                    best.left = left;
                }
            }
        }
    }
    return best;
}

std::vector<Candidate> ranked_candidates(const std::string& board) {
    std::array<int, 256> counts{};
    for (char ch : board) counts[static_cast<unsigned char>(ch)]++;

    std::vector<Candidate> out;
    for (int ch = 0; ch < 256; ++ch) {
        if (counts[ch] < 9) continue;
        char color = static_cast<char>(ch);
        for (int top = 0; top <= ROWS - 3; ++top) {
            for (int left = 0; left <= COLS - 3; ++left) {
                Candidate candidate;
                candidate.color = color;
                candidate.top = top;
                candidate.left = left;
                candidate.cost = assign_cost_for_window(board, color, top, left);
                out.push_back(candidate);
            }
        }
    }
    std::sort(out.begin(), out.end(), [](const Candidate& a, const Candidate& b) {
        return a.cost < b.cost;
    });
    return out;
}

int heuristic_cost(const std::string& board) {
    if (has_3x3(board)) return 0;
    return best_candidate(board).cost;
}

std::vector<int> target_cells(const Candidate& target) {
    std::vector<int> cells;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) cells.push_back(pos_of(target.top + r, target.left + c));
    }
    return cells;
}

HeuristicContext make_context(const Candidate& target) {
    HeuristicContext ctx;
    ctx.target = target;
    ctx.cells = target_cells(target);
    for (int j = 0; j < 9; ++j) {
        for (int pos = 0; pos < SIZE; ++pos) {
            ctx.dist_to[j][pos] = std::abs(row_of(pos) - row_of(ctx.cells[j])) +
                                  std::abs(col_of(pos) - col_of(ctx.cells[j]));
        }
    }
    return ctx;
}

int fixed_h(const std::string& board, const HeuristicContext& ctx) {
    std::array<int, 1 << 9> dp;
    dp.fill(INF);
    dp[0] = 0;
    for (int pos = 0; pos < SIZE; ++pos) {
        if (board[pos] != ctx.target.color) continue;
        auto next = dp;
        for (int mask = 0; mask < (1 << 9); ++mask) {
            if (dp[mask] >= INF) continue;
            for (int j = 0; j < 9; ++j) {
                if (mask & (1 << j)) continue;
                int nmask = mask | (1 << j);
                next[nmask] = std::min(next[nmask], dp[mask] + ctx.dist_to[j][pos]);
            }
        }
        dp = next;
    }
    return dp[(1 << 9) - 1];
}

int finger_term(const std::string& board, int finger, const HeuristicContext& ctx) {
    int best = INF;
    for (int pos = 0; pos < SIZE; ++pos) {
        if (board[pos] != ctx.target.color) continue;
        bool already_in_target = false;
        for (int cell : ctx.cells) {
            if (pos == cell) {
                already_in_target = true;
                break;
            }
        }
        if (already_in_target) continue;
        int dist = std::abs(row_of(finger) - row_of(pos)) + std::abs(col_of(finger) - col_of(pos));
        best = std::min(best, dist);
    }
    return best == INF ? 0 : std::max(0, best - 1);
}

int fixed_total_h(const std::string& board, int finger, const HeuristicContext& ctx) {
    if (has_3x3(board)) return 0;
    return fixed_h(board, ctx) + finger_term(board, finger, ctx);
}

std::vector<int> bfs_path(int start, int goal, const std::vector<bool>& blocked) {
    std::array<int, SIZE> parent;
    parent.fill(-1);
    std::queue<int> q;
    if (blocked[start] || blocked[goal]) return {};
    parent[start] = start;
    q.push(start);
    while (!q.empty()) {
        int curr = q.front();
        q.pop();
        if (curr == goal) break;
        for (int next : neighbors(curr)) {
            if (blocked[next] || parent[next] != -1) continue;
            parent[next] = curr;
            q.push(next);
        }
    }
    if (parent[goal] == -1) return {};

    std::vector<int> path;
    for (int p = goal; p != start; p = parent[p]) path.push_back(p);
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

void move_blank_along(std::string& board, int& blank, const std::vector<int>& path, std::string& route) {
    for (size_t i = 1; i < path.size(); ++i) {
        int next = path[i];
        route.push_back(move_char(blank, next));
        std::swap(board[blank], board[next]);
        blank = next;
    }
}

bool move_orb_bfs(std::string& board,
                  int orb_from,
                  int goal,
                  int& blank_pos,
                  const std::vector<bool>& locked,
                  std::string& route) {
    if (orb_from == goal) return true;

    struct Node {
        int orb;
        int blank;
    };

    std::array<std::array<int, SIZE>, SIZE> parent;
    std::array<std::array<char, SIZE>, SIZE> pmove;
    for (auto& row : parent) row.fill(-1);
    for (auto& row : pmove) row.fill('?');

    std::queue<Node> q;
    parent[orb_from][blank_pos] = orb_from * SIZE + blank_pos;
    q.push({orb_from, blank_pos});
    int found_blank = -1;

    while (!q.empty()) {
        Node node = q.front();
        q.pop();
        if (node.orb == goal) {
            found_blank = node.blank;
            break;
        }
        for (int next : neighbors(node.blank)) {
            if (locked[next]) continue;
            int next_orb = (next == node.orb) ? node.blank : node.orb;
            if (parent[next_orb][next] != -1) continue;
            parent[next_orb][next] = node.orb * SIZE + node.blank;
            pmove[next_orb][next] = move_char(node.blank, next);
            q.push({next_orb, next});
        }
    }

    if (found_blank < 0) return false;

    std::string seq;
    int orb = goal;
    int blank = found_blank;
    while (!(orb == orb_from && blank == blank_pos)) {
        seq.push_back(pmove[orb][blank]);
        int prev = parent[orb][blank];
        orb = prev / SIZE;
        blank = prev % SIZE;
    }
    std::reverse(seq.begin(), seq.end());

    for (char mv : seq) {
        int next = apply_move(blank_pos, mv);
        std::swap(board[blank_pos], board[next]);
        route.push_back(mv);
        blank_pos = next;
    }
    return true;
}

bool move_two_orbs_bfs(std::string& board,
                       char color,
                       int goal1,
                       int goal2,
                       int& blank_pos,
                       const std::vector<bool>& locked,
                       std::string& route) {
    if (board[goal1] == color && board[goal2] == color) return true;

    std::vector<int> orbs;
    for (int pos = 0; pos < SIZE; ++pos) {
        if (locked[pos] || pos == blank_pos || board[pos] != color) continue;
        orbs.push_back(pos);
    }
    if (orbs.size() < 2) return false;

    auto encode = [](int orb1, int orb2, int blank) {
        return (orb1 * SIZE + orb2) * SIZE + blank;
    };

    for (size_t a = 0; a < orbs.size(); ++a) {
        for (size_t b = a + 1; b < orbs.size(); ++b) {
            int start1 = orbs[a];
            int start2 = orbs[b];
            int start_id = encode(start1, start2, blank_pos);
            std::unordered_map<int, int> parent;
            std::unordered_map<int, char> pmove;
            std::queue<int> q;
            parent[start_id] = start_id;
            q.push(start_id);

            int found = -1;
            while (!q.empty()) {
                int id = q.front();
                q.pop();
                int blank = id % SIZE;
                int tmp = id / SIZE;
                int orb2 = tmp % SIZE;
                int orb1 = tmp / SIZE;

                bool done = (orb1 == goal1 && orb2 == goal2) || (orb1 == goal2 && orb2 == goal1);
                if (done) {
                    found = id;
                    break;
                }

                for (int next : neighbors(blank)) {
                    if (locked[next]) continue;
                    int next_orb1 = (next == orb1) ? blank : orb1;
                    int next_orb2 = (next == orb2) ? blank : orb2;
                    if (next_orb1 == next_orb2) continue;
                    int next_id = encode(next_orb1, next_orb2, next);
                    if (parent.find(next_id) != parent.end()) continue;
                    parent[next_id] = id;
                    pmove[next_id] = move_char(blank, next);
                    q.push(next_id);
                }
            }

            if (found < 0) continue;

            std::string seq;
            for (int id = found; id != start_id; id = parent[id]) {
                seq.push_back(pmove[id]);
            }
            std::reverse(seq.begin(), seq.end());

            for (char mv : seq) {
                int next = apply_move(blank_pos, mv);
                std::swap(board[blank_pos], board[next]);
                route.push_back(mv);
                blank_pos = next;
            }
            return board[goal1] == color && board[goal2] == color;
        }
    }
    return false;
}

int pick_nearest_free_orb(const std::string& board,
                          char color,
                          int goal,
                          const std::vector<bool>& locked,
                          int blank) {
    int best = -1;
    int best_dist = INF;
    for (int pos = 0; pos < SIZE; ++pos) {
        if (locked[pos] || pos == blank || board[pos] != color) continue;
        int dist = std::abs(row_of(pos) - row_of(goal)) + std::abs(col_of(pos) - col_of(goal));
        if (dist < best_dist) {
            best_dist = dist;
            best = pos;
        }
    }
    return best;
}

bool run_constructive_for_target(std::string& board,
                                 const Candidate& target,
                                 int& blank,
                                 int& start_blank,
                                 std::string& route,
                                 std::string& fail_note) {
    auto targets = target_cells(target);
    std::vector<bool> in_window(SIZE, false);
    for (int cell : targets) in_window[cell] = true;

    blank = -1;
    for (int i = 0; i < SIZE; ++i) {
        if (!in_window[i] && board[i] != target.color) {
            blank = i;
            break;
        }
    }
    if (blank < 0) blank = 0;
    start_blank = blank;

    if (blank < 0) {
        for (int i = 0; i < SIZE; ++i) {
            if (!in_window[i]) {
                blank = i;
                break;
            }
        }
    }
    if (blank < 0) blank = 0;

    std::vector<std::vector<int>> orders = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8},
        {0, 3, 6, 1, 4, 7, 2, 5, 8},
        {2, 1, 0, 5, 4, 3, 8, 7, 6},
        {6, 3, 0, 7, 4, 1, 8, 5, 2},
    };

    for (const auto& order : orders) {
        std::string attempt_board = board;
        std::string attempt_route;
        int attempt_blank = blank;
        std::vector<bool> locked(SIZE, false);
        bool ok = true;

        for (int oi = 0; oi < 7; ++oi) {
            int goal = targets[order[oi]];
            if (attempt_board[goal] == target.color) {
                locked[goal] = true;
                continue;
            }
            int orb = pick_nearest_free_orb(attempt_board, target.color, goal, locked, attempt_blank);
            if (orb < 0) {
                fail_note = "no movable target orb";
                ok = false;
                break;
            }
            if (!move_orb_bfs(attempt_board, orb, goal, attempt_blank, locked, attempt_route)) {
                fail_note = "single-orb BFS blocked";
                ok = false;
                break;
            }
            if (attempt_board[goal] != target.color) {
                fail_note = "single-orb replay failed";
                ok = false;
                break;
            }
            locked[goal] = true;
        }

        if (!ok) continue;

        int goal1 = targets[order[7]];
        int goal2 = targets[order[8]];
        if (!move_two_orbs_bfs(attempt_board, target.color, goal1, goal2, attempt_blank, locked, attempt_route)) {
            fail_note = "two-orb BFS blocked";
            continue;
        }
        if (has_3x3(attempt_board)) {
            board = attempt_board;
            blank = attempt_blank;
            route += attempt_route;
            return true;
        }
        fail_note = "constructive final board has no 3x3";
    }
    return false;
}

SearchResult scheme_a_constructive(const std::string& input) {
    auto started = std::chrono::high_resolution_clock::now();
    auto candidates = ranked_candidates(input);
    if (candidates.empty()) {
        SearchResult result;
        result.ms = 0.0;
        result.final_board = input;
        result.note = "no color has 9 orbs";
        return result;
    }

    std::string last_note = "no candidate tried";
    for (size_t ci = 0; ci < std::min<size_t>(candidates.size(), 12); ++ci) {
        std::string board = input;
        std::string route;
        int blank = -1;
        int start_blank = -1;
        const Candidate& target = candidates[ci];
        if (!run_constructive_for_target(board, target, blank, start_blank, route, last_note)) {
            continue;
        }

        auto ended = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(ended - started).count();
        SearchResult result;
        result.success = true;
        result.steps = static_cast<int>(route.size());
        result.start = start_blank;
        result.ms = ms;
        result.route = route;
        result.final_board = board;
        result.note = std::string("target ") + target.color + " @(" + std::to_string(target.top) + "," +
                      std::to_string(target.left) + ")";
        return result;
    }

    auto ended = std::chrono::high_resolution_clock::now();
    SearchResult result;
    result.ms = std::chrono::duration<double, std::milli>(ended - started).count();
    result.final_board = input;
    result.note = last_note;
    return result;
}

uint64_t time_elapsed_ms(std::chrono::high_resolution_clock::time_point started) {
    auto now = std::chrono::high_resolution_clock::now();
    return static_cast<uint64_t>(std::chrono::duration<double, std::milli>(now - started).count());
}

SearchResult astar_search(const std::string& input, int time_limit_ms, double weight) {
    auto started = std::chrono::high_resolution_clock::now();
    Candidate target = best_candidate(input);
    HeuristicContext ctx = make_context(target);
    struct Node {
        std::string board;
        int curr;
        int start;
        int steps;
        int cost;
        int fixed_cost;
        int seq;
        std::string route;
    };
    struct Greater {
        bool operator()(const Node& a, const Node& b) const {
            if (a.cost != b.cost) return a.cost > b.cost;
            return a.seq > b.seq;
        }
    };

    auto state_key = [&](const std::string& board, int finger) {
        std::string key;
        key.reserve(SIZE + 1);
        for (char ch : board) key.push_back(ch == ctx.target.color ? '1' : '0');
        key.push_back(static_cast<char>('A' + finger));
        return key;
    };

    std::priority_queue<Node, std::vector<Node>, Greater> open;
    std::unordered_map<std::string, int> best_steps;
    int seq = 0;
    int fixed0 = fixed_h(input, ctx);
    for (int start = 0; start < SIZE; ++start) {
        std::string key = state_key(input, start);
        best_steps[key] = 0;
        int h0 = fixed0 + finger_term(input, start, ctx);
        open.push({input, start, start, 0, static_cast<int>(weight * h0), fixed0, seq++, ""});
    }

    while (!open.empty()) {
        if (time_elapsed_ms(started) >= static_cast<uint64_t>(time_limit_ms)) break;
        Node node = open.top();
        open.pop();
        if (has_3x3(node.board)) {
            auto ended = std::chrono::high_resolution_clock::now();
            SearchResult result;
            result.success = true;
            result.steps = node.steps;
            result.start = node.start;
            result.ms = std::chrono::duration<double, std::milli>(ended - started).count();
            result.route = node.route;
            result.final_board = node.board;
            result.note = weight == 1.0 ? "A*" : "weighted A*";
            return result;
        }
        if (node.steps >= 80) continue;

        for (int next : neighbors(node.curr)) {
            std::string board = node.board;
            char a = board[node.curr];
            char b = board[next];
            std::swap(board[node.curr], board[next]);
            int steps = node.steps + 1;
            std::string key = state_key(board, next);
            auto it = best_steps.find(key);
            if (it != best_steps.end() && it->second <= steps) continue;
            best_steps[key] = steps;
            int child_fixed = (a != ctx.target.color && b != ctx.target.color) ? node.fixed_cost : fixed_h(board, ctx);
            int h = child_fixed + finger_term(board, next, ctx);
            std::string route = node.route;
            route.push_back(move_char(node.curr, next));
            open.push({board, next, node.start, steps, steps + static_cast<int>(weight * h), child_fixed, seq++,
                       std::move(route)});
        }
    }

    auto ended = std::chrono::high_resolution_clock::now();
    SearchResult result;
    result.ms = std::chrono::duration<double, std::milli>(ended - started).count();
    result.final_board = input;
    result.note = "timeout/open exhausted";
    return result;
}

SearchResult scheme_b_astar(const std::string& input) {
    return astar_search(input, 10000, 4.0);
}

SearchResult scheme_c_beam_with_limit(const std::string& input, int time_limit_ms) {
    auto started = std::chrono::high_resolution_clock::now();
    Candidate target = best_candidate(input);
    HeuristicContext ctx = make_context(target);
    auto state_key = [&](const std::string& board, int finger) {
        std::string key;
        key.reserve(SIZE + 1);
        for (char ch : board) key.push_back(ch == ctx.target.color ? '1' : '0');
        key.push_back(static_cast<char>('A' + finger));
        return key;
    };

    std::vector<PathState> beam;
    int initial_fixed = fixed_h(input, ctx);
    for (int start = 0; start < SIZE; ++start) {
        int initial_h = initial_fixed + finger_term(input, start, ctx);
        beam.push_back({input, start, -1, 0, initial_h, initial_fixed, start, ""});
    }
    std::unordered_set<std::string> seen;

    constexpr int MAX_DEPTH = 60;
    constexpr int BEAM_WIDTH = 200;
    for (int depth = 0; depth < MAX_DEPTH; ++depth) {
        if (time_elapsed_ms(started) >= static_cast<uint64_t>(time_limit_ms)) break;
        std::vector<PathState> next_beam;
        next_beam.reserve(BEAM_WIDTH * 4);
        for (const auto& state : beam) {
            if (time_elapsed_ms(started) >= static_cast<uint64_t>(time_limit_ms)) break;
            if (has_3x3(state.board)) {
                auto ended = std::chrono::high_resolution_clock::now();
                SearchResult result;
                result.success = true;
                result.steps = state.steps;
                result.start = state.start;
                result.ms = std::chrono::duration<double, std::milli>(ended - started).count();
                result.route = state.route;
                result.final_board = state.board;
                result.note = "beam width 200";
                return result;
            }
            for (int next : neighbors(state.curr)) {
                if (next == state.prev) continue;
                PathState child = state;
                char a = child.board[child.curr];
                char b = child.board[next];
                std::swap(child.board[child.curr], child.board[next]);
                child.prev = child.curr;
                child.curr = next;
                child.steps++;
                int child_fixed = (a != ctx.target.color && b != ctx.target.color)
                                      ? state.fixed_cost
                                      : fixed_h(child.board, ctx);
                child.h = child_fixed + finger_term(child.board, child.curr, ctx);
                child.fixed_cost = child_fixed;
                child.route.push_back(move_char(child.prev, child.curr));
                std::string key = state_key(child.board, child.curr);
                if (!seen.insert(key).second) continue;
                next_beam.push_back(std::move(child));
            }
        }
        std::sort(next_beam.begin(), next_beam.end(), [](const PathState& a, const PathState& b) {
            int ca = a.h * 100 + a.steps;
            int cb = b.h * 100 + b.steps;
            return ca < cb;
        });
        if (next_beam.size() > BEAM_WIDTH) next_beam.resize(BEAM_WIDTH);
        beam.swap(next_beam);
        if (beam.empty()) break;
    }

    auto ended = std::chrono::high_resolution_clock::now();
    SearchResult result;
    result.ms = std::chrono::duration<double, std::milli>(ended - started).count();
    result.final_board = input;
    result.note = "beam exhausted/timeout";
    return result;
}

SearchResult scheme_c_beam(const std::string& input) {
    return scheme_c_beam_with_limit(input, 10000);
}

SearchResult scheme_d_grasp(const std::string& input) {
    auto started = std::chrono::high_resolution_clock::now();
    Candidate target = best_candidate(input);
    HeuristicContext ctx = make_context(target);
    std::mt19937 rng(20260707);
    constexpr int RESTARTS = 80;
    constexpr int MAX_STEPS = 60;
    constexpr int TIME_LIMIT_MS = 10000;

    for (int restart = 0; restart < RESTARTS; ++restart) {
        if (time_elapsed_ms(started) >= TIME_LIMIT_MS) break;
        std::string board = input;
        int curr = restart % SIZE;
        int start = curr;
        int prev = -1;
        std::string route;
        for (int step = 0; step <= MAX_STEPS; ++step) {
            if (time_elapsed_ms(started) >= TIME_LIMIT_MS) break;
            if (has_3x3(board)) {
                auto ended = std::chrono::high_resolution_clock::now();
                SearchResult result;
                result.success = true;
                result.steps = step;
                result.start = start;
                result.ms = std::chrono::duration<double, std::milli>(ended - started).count();
                result.route = route;
                result.final_board = board;
                result.note = "GRASP restarts 80";
                return result;
            }

            struct Move {
                int next;
                int cost;
            };
            std::vector<Move> moves;
            for (int next : neighbors(curr)) {
                if (next == prev) continue;
                std::string candidate = board;
                std::swap(candidate[curr], candidate[next]);
                int cost = fixed_total_h(candidate, next, ctx);
                moves.push_back({next, cost});
            }
            if (moves.empty()) break;
            std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) {
                return a.cost < b.cost;
            });
            int pick_count = std::min<int>(3, moves.size());
            std::uniform_int_distribution<int> dist(0, pick_count - 1);
            int next = moves[dist(rng)].next;
            route.push_back(move_char(curr, next));
            std::swap(board[curr], board[next]);
            prev = curr;
            curr = next;
        }
    }

    auto ended = std::chrono::high_resolution_clock::now();
    SearchResult result;
    result.ms = std::chrono::duration<double, std::milli>(ended - started).count();
    result.final_board = input;
    result.note = "no rollout reached goal/timeout";
    return result;
}

SearchResult scheme_e_pipeline(const std::string& input) {
    auto started = std::chrono::high_resolution_clock::now();
    SearchResult fast = astar_search(input, 10000, 4.0);
    if (fast.success) {
        auto ended = std::chrono::high_resolution_clock::now();
        fast.ms = std::chrono::duration<double, std::milli>(ended - started).count();
        fast.note = "weighted A* phase";
        return fast;
    }
    SearchResult fallback = scheme_a_constructive(input);
    auto ended = std::chrono::high_resolution_clock::now();
    fallback.ms = std::chrono::duration<double, std::milli>(ended - started).count();
    fallback.note = "fallback A after weighted A*";
    return fallback;
}

void print_board(const std::string& board) {
    for (int r = 0; r < ROWS; ++r) {
        std::cout << "  ";
        for (int c = 0; c < COLS; ++c) std::cout << board[pos_of(r, c)];
        std::cout << "\n";
    }
}

bool can_place_without_line3(const std::string& board, int index, char color) {
    int row = row_of(index);
    int col = col_of(index);
    if (col >= 2 && board[index - 1] == color && board[index - 2] == color) return false;
    if (row >= 2 && board[index - COLS] == color && board[index - 2 * COLS] == color) return false;
    return true;
}

std::string generate_random_board(std::mt19937& rng) {
    const std::array<char, 6> colors{{'R', 'B', 'G', 'D', 'L', 'H'}};
    std::array<int, 6> remaining{{9, 5, 4, 4, 4, 4}};
    std::string board(SIZE, '?');

    std::function<bool(int)> fill = [&](int index) {
        if (index == SIZE) return true;

        std::vector<int> options;
        for (int i = 0; i < static_cast<int>(colors.size()); ++i) {
            if (remaining[i] > 0 && can_place_without_line3(board, index, colors[i])) {
                options.push_back(i);
            }
        }
        std::shuffle(options.begin(), options.end(), rng);
        std::sort(options.begin(), options.end(), [&](int a, int b) {
            return remaining[a] > remaining[b];
        });

        for (int option : options) {
            board[index] = colors[option];
            --remaining[option];
            if (fill(index + 1)) return true;
            ++remaining[option];
            board[index] = '?';
        }
        return false;
    };

    if (!fill(0)) return "";
    return board;
}

bool validate_success(const std::string& input, const std::string& name, const SearchResult& result) {
    std::string error;
    if (!replay_route(input, result, &error)) {
        std::cerr << name << " route validation failed: " << error << "\n";
        std::cerr << "input=" << input << " start=" << result.start << " route=" << result.route << "\n";
        return false;
    }
    return true;
}

bool run_stress_test(int cases) {
    constexpr int SEARCH_LIMIT_MS = 2000;
    std::mt19937 rng(20260707);

    int a_ok = 0;
    int b_ok = 0;
    int c_ok = 0;
    double a_ms = 0.0;
    double b_ms = 0.0;
    double c_ms = 0.0;

    for (int i = 0; i < cases; ++i) {
        std::string board = generate_random_board(rng);
        if (board.size() != SIZE || has_initial_line3(board)) {
            std::cerr << "Generated invalid board at case " << i << ": " << board << "\n";
            return false;
        }

        SearchResult a = scheme_a_constructive(board);
        a_ms += a.ms;
        if (!a.success || !validate_success(board, "stress A", a)) {
            std::cerr << "A failed at case " << i << ": " << board << "\n";
            return false;
        }
        ++a_ok;

        SearchResult b = astar_search(board, SEARCH_LIMIT_MS, 4.0);
        b_ms += b.ms;
        if (b.success) {
            if (!validate_success(board, "stress B", b)) return false;
            ++b_ok;
        }

        SearchResult c = scheme_c_beam_with_limit(board, SEARCH_LIMIT_MS);
        c_ms += c.ms;
        if (c.success) {
            if (!validate_success(board, "stress C", c)) return false;
            ++c_ok;
        }

        if ((i + 1) % 100 == 0) {
            std::cout << "Stress progress " << (i + 1) << "/" << cases << ": A=" << a_ok
                      << ", B=" << b_ok << ", C=" << c_ok << std::endl;
        }
    }

    std::cout << "\n=== Random stress test ===\n";
    std::cout << "Cases: " << cases << ", no initial horizontal/vertical 3-match, R=9\n";
    std::cout << "A success: " << a_ok << "/" << cases << ", avg_ms=" << (a_ms / cases) << "\n";
    std::cout << "B success: " << b_ok << "/" << cases << ", avg_ms=" << (b_ms / cases)
              << ", per-board limit_ms=" << SEARCH_LIMIT_MS << "\n";
    std::cout << "C success: " << c_ok << "/" << cases << ", avg_ms=" << (c_ms / cases)
              << ", per-board limit_ms=" << SEARCH_LIMIT_MS << "\n";

    if (a_ok != cases) return false;
    if (b_ok * 100 < cases * 95) return false;
    if (c_ok * 100 < cases * 95) return false;
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::vector<std::pair<std::string, std::string>> demos = {
        {"Legal cost 7", "RRHRRBLRRBGDLHDRHBBGRGDDLRGBHL"},
        {"Legal cost 9", "RRBRRBGRRHDGLHRBDLRGLBHDDLGHRB"},
        {"Legal cost 11", "RRBRRBDHRGLBGRHRDDLRLHBGBGDHLR"},
        {"Legal cost 13", "RRBRRBLDHRGRGDBHLRDBLHGLRBRGHD"},
        {"Legal cost 15", "RRBRRBRDRLGHDBGLLHRGHDBRBDGHLR"},
    };

    bool run_demos = true;
    bool compare_all = false;
    int stress_cases = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--stress") {
            if (i + 1 >= argc) {
                std::cerr << "--stress requires a case count\n";
                return 1;
            }
            stress_cases = std::atoi(argv[++i]);
            if (stress_cases <= 0) {
                std::cerr << "--stress case count must be positive\n";
                return 1;
            }
        } else if (arg == "--stress-only") {
            run_demos = false;
        } else if (arg == "--compare") {
            compare_all = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: benchmark_3x3_schemes.exe [--compare] [--stress N] [--stress-only]\n";
            std::cout << "Default demo route: B weighted A*. Use --compare to run A-E.\n";
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    if (!run_demos && stress_cases == 0) {
        std::cerr << "--stress-only requires --stress N\n";
        return 1;
    }

    if (run_demos) {
    for (const auto& demo : demos) {
        if (demo.second.size() != SIZE || has_initial_line3(demo.second)) {
            std::cerr << "Invalid demo board: " << demo.first
                      << " must be 30 cells and contain no horizontal/vertical 3-match.\n";
            return 1;
        }
        std::cout << "\n=== " << demo.first << " ===\n";
        std::cout << "Input: " << demo.second << "\n";
        print_board(demo.second);
        Candidate candidate = best_candidate(demo.second);
        std::cout << "Best target: color=" << candidate.color << " window=(" << candidate.top << ","
                  << candidate.left << ") assignment_cost=" << candidate.cost << "\n";

        std::vector<std::pair<std::string, SearchResult (*)(const std::string&)>> schemes = {
            {"B A*", scheme_b_astar},
        };
        if (compare_all) {
            schemes = {
                {"A constructive", scheme_a_constructive},
                {"B A*", scheme_b_astar},
                {"C beam", scheme_c_beam},
                {"D GRASP", scheme_d_grasp},
                {"E pipeline", scheme_e_pipeline},
            };
        }

        for (const auto& scheme : schemes) {
            SearchResult result = scheme.second(demo.second);
            if (result.success && !validate_success(demo.second, scheme.first, result)) {
                return 1;
            }
            std::cout << scheme.first << ": " << (result.success ? "OK" : "FAIL")
                      << ", steps=" << result.steps << ", time_ms=" << result.ms
                      << ", note=" << result.note << "\n";
        }
    }
    }

    if (stress_cases > 0 && !run_stress_test(stress_cases)) {
        std::cerr << "Stress test failed.\n";
        return 1;
    }

    return 0;
}
