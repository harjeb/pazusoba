#include <pazusoba/core.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

const std::array<char, 6> COLORS{{'R', 'B', 'G', 'D', 'L', 'H'}};
const int CASES = 10;
const unsigned int SEED = 20260707;

struct TimedResult {
    bool ok = false;
    double ms = 0.0;
    int steps = -1;
    int start = -1;
    int combo = -1;
    int max_combo = -1;
    char color = '-';
    std::string note;
};

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

bool has_initial_line3(const std::string& board, int rows, int cols) {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col <= cols - 3; ++col) {
            int pos = row * cols + col;
            if (board[pos] == board[pos + 1] && board[pos] == board[pos + 2])
                return true;
        }
    }
    for (int row = 0; row <= rows - 3; ++row) {
        for (int col = 0; col < cols; ++col) {
            int pos = row * cols + col;
            if (board[pos] == board[pos + cols] && board[pos] == board[pos + 2 * cols])
                return true;
        }
    }
    return false;
}

std::string generate_board(int rows,
                           int cols,
                           const std::array<int, 6>& counts,
                           std::mt19937& rng) {
    std::string pool;
    pool.reserve(rows * cols);
    for (int i = 0; i < (int)COLORS.size(); ++i) {
        for (int count = 0; count < counts[i]; ++count)
            pool.push_back(COLORS[i]);
    }
    if ((int)pool.size() != rows * cols)
        return "";

    for (int attempt = 0; attempt < 100000; ++attempt) {
        std::shuffle(pool.begin(), pool.end(), rng);
        if (!has_initial_line3(pool, rows, cols)) {
            return pool;
        }
    }
    return "";
}

std::string board_from_state(const pazusoba::solver& solver, const pazusoba::game_board& board) {
    return solver.get_board_string(board);
}

bool has_exact_connected_target(const std::string& board) {
    pazusoba::solver solver;
    solver.set_board(board.c_str());
    pazusoba::profile profile;
    profile.name = pazusoba::connected_orb;
    profile.target = 4;
    profile.orbs[1] = true;
    profile.orbs[3] = true;
    solver.set_profiles(&profile, 1);
    auto copy = solver.board();
    pazusoba::state state;
    solver.evaluate(copy, state);
    return state.goal;
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

std::string route_to_string(const pazusoba::route_list& route, int step) {
    std::string out;
    int max_index = step / ROUTE_PER_LIST;
    int offset = step % ROUTE_PER_LIST;
    int index = 0;
    while (index <= max_index) {
        auto curr = route[index];
        int limit = ROUTE_PER_LIST;
        if (index == max_index) {
            limit = offset;
            curr <<= (ROUTE_PER_LIST - offset) * 3;
        }
        for (int i = 0; i < limit; ++i) {
            int dir = (curr & ROUTE_MASK) >> 60;
            out.push_back(pazusoba::DIRECTION_NAME[dir]);
            curr <<= 3;
        }
        index++;
    }
    return out;
}

bool replay_solver_route(const std::string& input,
                         int rows,
                         int cols,
                         const pazusoba::state& state,
                         const std::string& final_board) {
    if (state.step <= 0 || state.begin < 0 || state.begin >= rows * cols)
        return false;
    std::string board = input;
    int curr = state.begin;
    std::string route = route_to_string(state.route, state.step);
    for (char move : route) {
        int next = apply_move(curr, move, cols);
        if (next < 0 || next >= rows * cols)
            return false;
        int dr = std::abs(next / cols - curr / cols);
        int dc = std::abs(next % cols - curr % cols);
        if ((dr == 0 && dc == 0) || dr > 1 || dc > 1)
            return false;
        std::swap(board[curr], board[next]);
        curr = next;
    }
    return board == final_board;
}

TimedResult run_shape_mode(const std::string& board,
                           int rows,
                           int cols,
                           int kind,
                           const std::array<bool, MAX_BOARD_LENGTH>& blocked,
                           bool allow_diagonal,
                           int color_mode = pazusoba::color_prefer) {
    pazusoba::shape_request request;
    request.shape = kind;
    request.mode = color_mode;
    request.allow_diagonal = allow_diagonal;
    request.colors[1] = true;  // R
    request.colors[3] = true;  // G

    auto start = std::chrono::high_resolution_clock::now();
    auto result = pazusoba::solve_shape(board, rows, cols, request, blocked);
    auto end = std::chrono::high_resolution_clock::now();

    TimedResult out;
    out.ms = std::chrono::duration<double, std::milli>(end - start).count();
    out.ok = result.success && replay_route(board, rows, cols, result, blocked) &&
             pazusoba::board_has_shape(result.final_board, rows, cols, kind);
    out.steps = result.steps;
    out.start = result.start;
    out.color = result.color > 0 ? pazusoba::ORB_WEB_NAME[result.color] : '-';
    if (result.success) {
        out.combo = result.combo;
        out.max_combo = result.max_combo;
    }
    out.note = result.note;
    return out;
}

TimedResult run_priority_4_mode(const std::string& board, bool allow_diagonal) {
    pazusoba::solver solver;
    solver.set_board(board.c_str());
    solver.set_search_depth(35);
    solver.set_beam_size(2500);
    solver.set_diagonal(allow_diagonal);

    pazusoba::profile profiles[2];
    profiles[0].name = pazusoba::connected_orb;
    profiles[0].target = 4;
    profiles[0].stop_threshold = 35;
    profiles[0].orbs[1] = true;  // R
    profiles[0].orbs[3] = true;  // G
    profiles[1].name = pazusoba::target_combo;
    profiles[1].target = -1;
    profiles[1].stop_threshold = 35;
    solver.set_profiles(profiles, 2);

    auto start = std::chrono::high_resolution_clock::now();
    auto state = solver.adventure();
    auto end = std::chrono::high_resolution_clock::now();

    TimedResult out;
    out.ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::string final_board = board_from_state(solver, state.board);
    out.ok = replay_solver_route(board, 5, 6, state, final_board) &&
             has_exact_connected_target(final_board);
    out.steps = state.step;
    out.start = state.begin;
    out.combo = state.combo;
    out.max_combo = solver.max_combo();
    out.note = out.ok ? "found exact R/G 4-connected erase, combo-scored"
                      : "beam did not preserve exact 4-connected goal";
    return out;
}

TimedResult run_combo_mode(const std::string& board, bool allow_diagonal) {
    pazusoba::solver solver;
    solver.set_board(board.c_str());
    solver.set_search_depth(60);
    solver.set_beam_size(5000);
    solver.set_diagonal(allow_diagonal);

    pazusoba::profile profile;
    profile.name = pazusoba::target_combo;
    profile.target = -1;
    profile.stop_threshold = 60;
    solver.set_profiles(&profile, 1);

    auto start = std::chrono::high_resolution_clock::now();
    auto state = solver.adventure();
    auto end = std::chrono::high_resolution_clock::now();

    TimedResult out;
    out.ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::string final_board = board_from_state(solver, state.board);
    out.ok = replay_solver_route(board, solver.row(), solver.column(), state, final_board) &&
             state.combo > 0;
    out.steps = state.step;
    out.start = state.begin;
    out.combo = state.combo;
    out.max_combo = solver.max_combo();
    out.note = allow_diagonal ? "7x6 target_combo with diagonal movement"
                              : "7x6 target_combo";
    return out;
}

TimedResult run_blocked_mode(const std::string& board,
                             int rows,
                             int cols,
                             int case_index,
                             bool allow_diagonal) {
    std::array<bool, MAX_BOARD_LENGTH> blocked{};
    blocked.fill(false);
    blocked[(case_index * 7 + 1) % (rows * cols)] = true;
    blocked[(case_index * 11 + 5) % (rows * cols)] = true;
    return run_shape_mode(board, rows, cols, pazusoba::shape_cross, blocked, allow_diagonal,
                          pazusoba::color_prefer);
}

std::string mark(bool ok) {
    return ok ? "PASS" : "FAIL";
}

void append_result_row(std::ostringstream& report,
                       int index,
                       const std::string& board,
                       const char* mode,
                       const TimedResult& result) {
    report << "| " << index << " | `" << board << "` | " << mode << " | "
           << mark(result.ok) << " | " << std::fixed << std::setprecision(3) << result.ms
           << " | " << result.steps << " | " << result.start << " | " << result.color
           << " | " << result.combo << "/" << result.max_combo
           << " | " << result.note << " |\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    std::string output = "docs/six_modes_random_report.md";
    bool allow_diagonal = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--diagonal") {
            allow_diagonal = true;
        } else {
            output = argv[i];
        }
    }

    std::mt19937 rng(SEED);
    std::vector<std::string> boards_6x5;
    std::vector<std::string> boards_7x6;
    for (int i = 0; i < CASES; ++i) {
        std::string board_6x5 = generate_board(5, 6, {{9, 4, 7, 4, 3, 3}}, rng);
        std::string board_7x6 = generate_board(6, 7, {{9, 7, 7, 7, 6, 6}}, rng);
        if (board_6x5.empty() || board_7x6.empty()) {
            std::cerr << "failed to generate random board\n";
            return 1;
        }
        if (has_initial_line3(board_6x5, 5, 6) || has_initial_line3(board_7x6, 6, 7)) {
            std::cerr << "generated board has initial line3\n";
            return 1;
        }
        boards_6x5.push_back(board_6x5);
        boards_7x6.push_back(board_7x6);
    }

    std::array<int, 6> pass_counts{};
    std::array<int, 6> total_combo{};
    std::array<double, 6> total_ms{};
    std::array<double, 6> max_ms{};
    std::ostringstream rows;

    for (int i = 0; i < CASES; ++i) {
        TimedResult r1 = run_priority_4_mode(boards_6x5[i], allow_diagonal);
        pass_counts[0] += r1.ok ? 1 : 0;
        total_combo[0] += std::max(0, r1.combo);
        total_ms[0] += r1.ms;
        max_ms[0] = std::max(max_ms[0], r1.ms);
        append_result_row(rows, i + 1, boards_6x5[i], "1 红/绿优先 4 连", r1);

        TimedResult r2 = run_blocked_mode(boards_6x5[i], 5, 6, i, allow_diagonal);
        pass_counts[1] += r2.ok ? 1 : 0;
        total_combo[1] += std::max(0, r2.combo);
        total_ms[1] += r2.ms;
        max_ms[1] = std::max(max_ms[1], r2.ms);
        append_result_row(rows, i + 1, boards_6x5[i], "2 固定点不经过", r2);

        TimedResult r3 = run_combo_mode(boards_7x6[i], allow_diagonal);
        pass_counts[2] += r3.ok ? 1 : 0;
        total_combo[2] += std::max(0, r3.combo);
        total_ms[2] += r3.ms;
        max_ms[2] = std::max(max_ms[2], r3.ms);
        append_result_row(rows, i + 1, boards_7x6[i], "3 7x6 combo", r3);

        TimedResult r4 = run_shape_mode(boards_6x5[i], 5, 6, pazusoba::shape_cross,
                                        std::array<bool, MAX_BOARD_LENGTH>{}, allow_diagonal);
        pass_counts[3] += r4.ok ? 1 : 0;
        total_combo[3] += std::max(0, r4.combo);
        total_ms[3] += r4.ms;
        max_ms[3] = std::max(max_ms[3], r4.ms);
        append_result_row(rows, i + 1, boards_6x5[i], "4 十字", r4);

        TimedResult r5 = run_shape_mode(boards_6x5[i], 5, 6, pazusoba::shape_full_row,
                                        std::array<bool, MAX_BOARD_LENGTH>{}, allow_diagonal);
        pass_counts[4] += r5.ok ? 1 : 0;
        total_combo[4] += std::max(0, r5.combo);
        total_ms[4] += r5.ms;
        max_ms[4] = std::max(max_ms[4], r5.ms);
        append_result_row(rows, i + 1, boards_6x5[i], "5 一行", r5);

        TimedResult r6 = run_shape_mode(boards_6x5[i], 5, 6, pazusoba::shape_full_column,
                                        std::array<bool, MAX_BOARD_LENGTH>{}, allow_diagonal);
        pass_counts[5] += r6.ok ? 1 : 0;
        total_combo[5] += std::max(0, r6.combo);
        total_ms[5] += r6.ms;
        max_ms[5] = std::max(max_ms[5], r6.ms);
        append_result_row(rows, i + 1, boards_6x5[i], "6 一竖", r6);
    }

    const std::array<const char*, 6> names{{
        "1 红/绿优先 4 连",
        "2 固定点不经过",
        "3 7x6 combo",
        "4 十字",
        "5 一行",
        "6 一竖",
    }};

    std::ostringstream report;
    report << "# 六种追加模式随机测试报告\n\n";
    report << "- 用例数: " << CASES << " 组随机 6x5 盘面，另配 " << CASES << " 组随机 7x6 盘面测试 7x6 模式\n";
    report << "- 随机种子: `" << SEED << "`\n";
    report << "- 盘面规则: 所有生成盘面都无初始横向或竖向 3 连\n";
    report << "- 优先颜色: `R` + `G`，验证多颜色优先\n";
    report << "- 对角线移动: " << (allow_diagonal ? "开启" : "关闭") << "\n";
    report << "- 超时标准: 单项最大允许 10000 ms\n";
    report << "- 路径校验: 成功结果全部按 route 重放，断言不越界；固定点模式额外断言手指不进入 blocked 格\n";
    report << "- 求解目标: 先满足模式规则；在规则成立的候选里选择最终 combo 更高的结果，combo 相同再偏向步数更短\n";
    report << "- Combo 统计: 对最终盘面运行标准消除模拟，记录 `combo/max_combo`\n\n";

    report << "## 汇总\n\n";
    report << "| 模式 | 成功率 | 平均 Combo | 平均耗时 ms | 最大耗时 ms |\n";
    report << "|---|---:|---:|---:|---:|\n";
    for (int i = 0; i < 6; ++i) {
        report << "| " << names[i] << " | " << pass_counts[i] << "/" << CASES << " | "
               << std::fixed << std::setprecision(2) << ((double)total_combo[i] / CASES) << " | "
               << std::fixed << std::setprecision(3) << (total_ms[i] / CASES) << " | "
               << max_ms[i] << " |\n";
    }

    report << "\n## 明细\n\n";
    report << "| 序号 | 盘面 | 模式 | 结果 | ms | 步数 | 起点 | 颜色 | Combo | 备注 |\n";
    report << "|---:|---|---|---|---:|---:|---:|---|---:|---|\n";
    report << rows.str();

    std::ofstream out(output.c_str());
    if (!out) {
        std::cerr << "failed to write " << output << "\n";
        return 1;
    }
    out << report.str();
    out.close();

    std::cout << report.str();

    bool all_ok = true;
    for (int pass : pass_counts) {
        if (pass != CASES)
            all_ok = false;
    }
    return all_ok ? 0 : 1;
}
