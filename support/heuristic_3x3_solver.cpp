#include <pazusoba/core.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

using namespace pazusoba;

struct Square3x3Target {
    int top_row, top_col;
    int orb_type;
    int current_matches;
    double reachability_score;
    std::vector<int> missing_positions;
    std::vector<int> wrong_orbs_in_area;
};

class Heuristic3x3Solver {
private:
    static constexpr int ROWS = 5;
    static constexpr int COLS = 6;
    static constexpr int BOARD_SIZE = 30;
    
public:
    // 分析所有可能的3x3目标位置
    std::vector<Square3x3Target> analyze_all_3x3_targets(const game_board& board) {
        std::vector<Square3x3Target> targets;
        
        // 统计每种珠子的数量
        int orb_counts[ORB_COUNT] = {0};
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (board[i] > 0) orb_counts[board[i]]++;
        }
        
        // 只考虑有9+珠子的颜色
        for (int orb_type = 1; orb_type < ORB_COUNT; orb_type++) {
            if (orb_counts[orb_type] < 9) continue;
            
            // 检查所有可能的3x3位置
            for (int top_row = 0; top_row <= ROWS - 3; top_row++) {
                for (int top_col = 0; top_col <= COLS - 3; top_col++) {
                    Square3x3Target target = analyze_3x3_target(board, top_row, top_col, orb_type);
                    if (target.reachability_score > 0) {
                        targets.push_back(target);
                    }
                }
            }
        }
        
        // 按可达性分数排序
        std::sort(targets.begin(), targets.end(), 
                  [](const Square3x3Target& a, const Square3x3Target& b) {
                      return a.reachability_score > b.reachability_score;
                  });
        
        return targets;
    }
    
    // 分析特定3x3位置的可达性
    Square3x3Target analyze_3x3_target(const game_board& board, int top_row, int top_col, int orb_type) {
        Square3x3Target target;
        target.top_row = top_row;
        target.top_col = top_col;
        target.orb_type = orb_type;
        target.current_matches = 0;
        target.reachability_score = 0;
        
        std::vector<int> target_positions;
        std::vector<int> current_orb_positions;
        
        // 收集目标区域信息
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int pos = (top_row + i) * COLS + (top_col + j);
                target_positions.push_back(pos);
                
                if (board[pos] == orb_type) {
                    target.current_matches++;
                } else {
                    target.missing_positions.push_back(pos);
                    if (board[pos] != 0) {
                        target.wrong_orbs_in_area.push_back(pos);
                    }
                }
            }
        }
        
        // 找到所有目标珠子的位置
        for (int i = 0; i < BOARD_SIZE; i++) {
            if (board[i] == orb_type) {
                current_orb_positions.push_back(i);
            }
        }
        
        // 计算可达性分数
        target.reachability_score = calculate_reachability_score(board, target, current_orb_positions);
        
        return target;
    }
    
    // 计算3x3目标的可达性分数
    double calculate_reachability_score(const game_board& board, 
                                      const Square3x3Target& target, 
                                      const std::vector<int>& orb_positions) {
        double score = 0;
        
        // 基础分数：已经匹配的珠子
        score += target.current_matches * 100;
        
        // 距离分析：计算最近的未使用珠子到缺失位置的距离
        for (int missing_pos : target.missing_positions) {
            double min_distance = 1000;
            
            for (int orb_pos : orb_positions) {
                // 跳过已经在目标区域的珠子
                bool already_in_target = false;
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        int target_pos = (target.top_row + i) * COLS + (target.top_col + j);
                        if (orb_pos == target_pos && board[orb_pos] == target.orb_type) {
                            already_in_target = true;
                            break;
                        }
                    }
                    if (already_in_target) break;
                }
                
                if (!already_in_target) {
                    double distance = calculate_move_distance(orb_pos, missing_pos);
                    min_distance = std::min(min_distance, distance);
                }
            }
            
            // 距离越近分数越高
            if (min_distance < 1000) {
                score += 50 / (1 + min_distance);
            }
        }
        
        // 障碍物惩罚：目标区域内的错误珠子
        score -= target.wrong_orbs_in_area.size() * 20;
        
        // 聚集度奖励：已匹配珠子的聚集程度
        score += calculate_clustering_bonus(target);
        
        // 边界奖励：靠近边界的3x3更容易形成
        if (target.top_row == 0 || target.top_col == 0 || 
            target.top_row == ROWS - 3 || target.top_col == COLS - 3) {
            score += 30;
        }
        
        return score;
    }
    
    // 计算两个位置之间的移动距离（曼哈顿距离）
    double calculate_move_distance(int pos1, int pos2) {
        int row1 = pos1 / COLS, col1 = pos1 % COLS;
        int row2 = pos2 / COLS, col2 = pos2 % COLS;
        return abs(row1 - row2) + abs(col1 - col2);
    }
    
    // 计算聚集度奖励
    double calculate_clustering_bonus(const Square3x3Target& target) {
        if (target.current_matches <= 1) return 0;
        
        // 检查连续区域
        int max_connected = 0;
        // 简化版：如果相邻位置都有匹配，给予额外奖励
        
        // 检查是否有连续的行
        for (int i = 0; i < 3; i++) {
            int row_matches = 0;
            for (int j = 0; j < 3; j++) {
                // 这里需要实际检查board，但为了简化，我们用current_matches估算
                // 实际实现中应该检查每个位置
            }
        }
        
        return target.current_matches >= 4 ? 20 : 0;
    }
    
    // 评估移动对3x3目标的影响
    double evaluate_move_for_3x3(const game_board& board, 
                                int from_pos, int to_pos, 
                                const Square3x3Target& target) {
        double score = 0;
        
        int from_row = from_pos / COLS, from_col = from_pos % COLS;
        int to_row = to_pos / COLS, to_col = to_pos % COLS;
        
        // 检查移动是否涉及目标区域
        bool from_in_target = (from_row >= target.top_row && from_row < target.top_row + 3 &&
                              from_col >= target.top_col && from_col < target.top_col + 3);
        bool to_in_target = (to_row >= target.top_row && to_row < target.top_row + 3 &&
                            to_col >= target.top_col && to_col < target.top_col + 3);
        
        // 如果把目标珠子移入目标区域
        if (!from_in_target && to_in_target && board[from_pos] == target.orb_type) {
            score += 1000;
        }
        
        // 如果把目标珠子移出目标区域
        if (from_in_target && !to_in_target && board[from_pos] == target.orb_type) {
            score -= 800;
        }
        
        // 如果把错误珠子移出目标区域
        if (from_in_target && !to_in_target && board[from_pos] != target.orb_type) {
            score += 300;
        }
        
        // 如果把错误珠子移入目标区域
        if (!from_in_target && to_in_target && board[from_pos] != target.orb_type) {
            score -= 500;
        }
        
        return score;
    }
    
    void print_3x3_analysis(const game_board& board) {
        auto targets = analyze_all_3x3_targets(board);
        
        printf("\n=== 3X3 HEURISTIC ANALYSIS ===\n");
        printf("Found %d potential 3x3 targets:\n\n", (int)targets.size());
        
        for (int i = 0; i < std::min(5, (int)targets.size()); i++) {
            const auto& target = targets[i];
            printf("Target %d: Orb %d at (%d,%d)\n", i+1, target.orb_type, 
                   target.top_row, target.top_col);
            printf("  Current matches: %d/9\n", target.current_matches);
            printf("  Reachability score: %.1f\n", target.reachability_score);
            printf("  Missing positions: %d\n", (int)target.missing_positions.size());
            printf("  Wrong orbs in area: %d\n", (int)target.wrong_orbs_in_area.size());
            
            // 显示目标区域
            printf("  Target area:\n");
            for (int row = 0; row < 3; row++) {
                printf("    ");
                for (int col = 0; col < 3; col++) {
                    int pos = (target.top_row + row) * COLS + (target.top_col + col);
                    char orb = board[pos] == 0 ? '.' : 'A' + board[pos] - 1;
                    if (board[pos] == target.orb_type) {
                        printf("[%c]", orb);
                    } else {
                        printf(" %c ", orb);
                    }
                }
                printf("\n");
            }
            printf("\n");
        }
    }
};

// 修改版的evaluate函数，使用3x3启发式
void heuristic_3x3_evaluate(game_board& board, state& new_state, solver& s, 
                            Heuristic3x3Solver& heuristic_solver) {
    short int score = 0;
    
    // 获取最佳3x3目标
    auto targets = heuristic_solver.analyze_all_3x3_targets(board);
    
    if (targets.empty()) {
        new_state.score = -1000;
        new_state.goal = false;
        new_state.combo = 0;
        return;
    }
    
    // 使用最佳目标的可达性分数作为主要分数
    const auto& best_target = targets[0];
    score = (int)best_target.reachability_score;
    
    // 检查是否已经达成3x3
    combo_list list;
    game_board copy = board;
    s.erase_combo(copy, list);
    
    bool found_3x3 = false;
    for (const auto& c : list) {
        if (c.loc.size() >= 9 && s.is_3x3_square(c.loc, s.column())) {
            score += 100000;  // 找到3x3的巨额奖励
            found_3x3 = true;
            break;
        }
    }
    
    new_state.score = score;
    new_state.goal = found_3x3;
    new_state.combo = list.size();
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <board> <min_erase> <max_steps> <beam_size>\n", argv[0]);
        return 1;
    }
    
    std::string board_str = argv[1];
    int min_erase = atoi(argv[2]);
    int max_steps = atoi(argv[3]);
    int beam_size = atoi(argv[4]);
    
    // 设置棋盘
    pazusoba::solver solver;
    game_board board = {0};
    for (int i = 0; i < std::min((int)board_str.length(), 30); i++) {
        char c = board_str[i];
        if (c == 'R') {
            board[i] = 1;  // R
        } else if (c == 'B') {
            board[i] = 2;  // B
        } else if (c == 'G') {
            board[i] = 3;  // G
        } else if (c == 'L') {
            board[i] = 4;  // L
        } else if (c == 'D') {
            board[i] = 5;  // D
        } else if (c == 'H') {
            board[i] = 6;  // H
        }
    }
    
    // 打印启发式分析
    Heuristic3x3Solver heuristic_solver;
    printf("Board: %s\n", board_str.c_str());
    heuristic_solver.print_3x3_analysis(board);
    
    return 0;
}