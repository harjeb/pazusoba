#include <pazusoba/core.h>

using namespace pazusoba;

// 全局的最佳3x3目标信息
struct Best3x3Target {
    int top_row, top_col;
    int orb_type;
    int target_score;
} g_best_target;

// 计算特定3x3位置的分数
int calculate_3x3_target_score(const game_board& board, int top_row, int top_col, int orb_type) {
    int score = 0;
    int matches = 0;
    int wrong_orbs = 0;
    
    // 统计目标区域情况
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int pos = (top_row + i) * 6 + (top_col + j);
            if (board[pos] == orb_type) {
                matches++;
            } else if (board[pos] != 0) {
                wrong_orbs++;
            }
        }
    }
    
    // 计算分数
    score += matches * 1000;  // 已匹配的珠子
    score -= wrong_orbs * 100; // 错误珠子的惩罚
    
    // 聚集度奖励
    if (matches >= 6) score += 2000;
    else if (matches >= 4) score += 500;
    
    // 边界奖励
    if (top_row == 0 || top_col == 0 || top_row == 2 || top_col == 3) {
        score += 200;
    }
    
    return score;
}

// 找到最佳的3x3目标
void find_best_3x3_target(const game_board& board) {
    int best_score = -1;
    g_best_target.target_score = -1;
    
    // 统计珠子数量
    int orb_counts[ORB_COUNT] = {0};
    for (int i = 0; i < 30; i++) {
        if (board[i] > 0) orb_counts[board[i]]++;
    }
    
    // 检查所有可能的3x3位置和珠子类型
    for (int orb_type = 1; orb_type < ORB_COUNT; orb_type++) {
        if (orb_counts[orb_type] < 9) continue;  // 需要至少9个珠子
        
        for (int top_row = 0; top_row <= 2; top_row++) {
            for (int top_col = 0; top_col <= 3; top_col++) {
                int score = calculate_3x3_target_score(board, top_row, top_col, orb_type);
                
                if (score > best_score) {
                    best_score = score;
                    g_best_target.top_row = top_row;
                    g_best_target.top_col = top_col;
                    g_best_target.orb_type = orb_type;
                    g_best_target.target_score = score;
                }
            }
        }
    }
}

// 评估移动对最佳3x3目标的贡献
int evaluate_move_contribution(const game_board& board, int from_pos, int to_pos) {
    if (g_best_target.target_score < 0) return 0;
    
    int score = 0;
    int from_row = from_pos / 6, from_col = from_pos % 6;
    int to_row = to_pos / 6, to_col = to_pos % 6;
    
    // 检查是否在目标3x3区域内
    bool from_in_target = (from_row >= g_best_target.top_row && from_row < g_best_target.top_row + 3 &&
                          from_col >= g_best_target.top_col && from_col < g_best_target.top_col + 3);
    bool to_in_target = (to_row >= g_best_target.top_row && to_row < g_best_target.top_row + 3 &&
                        to_col >= g_best_target.top_col && to_col < g_best_target.top_col + 3);
    
    // 评估移动的价值
    if (!from_in_target && to_in_target && board[from_pos] == g_best_target.orb_type) {
        // 将目标珠子移入目标区域
        score += 5000;
    } else if (from_in_target && !to_in_target && board[from_pos] == g_best_target.orb_type) {
        // 将目标珠子移出目标区域（很糟糕）
        score -= 3000;
    } else if (from_in_target && !to_in_target && board[from_pos] != g_best_target.orb_type) {
        // 将非目标珠子移出目标区域（好事）
        score += 1000;
    } else if (!from_in_target && to_in_target && board[from_pos] != g_best_target.orb_type) {
        // 将非目标珠子移入目标区域（不好）
        score -= 2000;
    }
    
    return score;
}

// 3x3启发式评估函数
void heuristic_3x3_evaluate(game_board& board, state& new_state) {
    short int score = 0;
    
    // 找到最佳3x3目标
    find_best_3x3_target(board);
    
    if (g_best_target.target_score < 0) {
        new_state.score = -10000;  // 没有可能的3x3目标
        new_state.goal = false;
        new_state.combo = 0;
        return;
    }
    
    // 基础分数来自最佳目标 - 不再缩放
    score += g_best_target.target_score;
    
    // 计算目标区域的当前状态
    int current_matches = 0;
    int total_distance = 0;
    int target_orb_count = 0;
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int pos = (g_best_target.top_row + i) * 6 + (g_best_target.top_col + j);
            if (board[pos] == g_best_target.orb_type) {
                current_matches++;
            }
        }
    }
    
    // 统计整个棋盘上的目标珠子数量
    for (int i = 0; i < 30; i++) {
        if (board[i] == g_best_target.orb_type) {
            target_orb_count++;
        }
    }
    
    // 计算所有目标珠子到目标区域的曼哈顿距离
    for (int i = 0; i < 30; i++) {
        if (board[i] == g_best_target.orb_type) {
            int row = i / 6, col = i % 6;
            bool in_target = (row >= g_best_target.top_row && row < g_best_target.top_row + 3 &&
                             col >= g_best_target.top_col && col < g_best_target.top_col + 3);
            
            if (!in_target) {
                // 计算到目标区域中心的距离
                int center_row = g_best_target.top_row + 1;
                int center_col = g_best_target.top_col + 1;
                int distance = abs(row - center_row) + abs(col - center_col);
                total_distance += distance;
            }
        }
    }
    
    // 根据当前匹配数给予渐进奖励
    if (current_matches >= 8) score += 50000;
    else if (current_matches >= 7) score += 30000;
    else if (current_matches >= 6) score += 15000;
    else if (current_matches >= 5) score += 8000;
    else if (current_matches >= 4) score += 4000;
    
    // 距离奖励 - 珠子越接近目标区域越好
    score += (1000 - total_distance * 10);
    
    // 密度奖励 - 目标珠子数量越多越好
    score += target_orb_count * 500;
    
    // 检查是否形成了完美3x3
    bool found_perfect_3x3 = (current_matches == 9);
    
    if (found_perfect_3x3) {
        score += 200000;  // 找到完美3x3的巨大奖励
        new_state.goal = true;
    } else {
        new_state.goal = false;
    }
    
    new_state.score = score;
    new_state.combo = current_matches / 3;  // 基于匹配数的combo
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <board> <min_erase> <max_steps> <beam_size>\n", argv[0]);
        return 1;
    }
    
    // 解析参数
    std::string board_str = argv[1];
    int min_erase = atoi(argv[2]);
    int max_steps = atoi(argv[3]);
    int beam_size = atoi(argv[4]);
    
    // 设置棋盘
    game_board board = {0};
    for (int i = 0; i < std::min((int)board_str.length(), 30); i++) {
        char c = board_str[i];
        if (c == 'R') board[i] = 1;
        else if (c == 'B') board[i] = 2;
        else if (c == 'G') board[i] = 3;
        else if (c == 'L') board[i] = 4;
        else if (c == 'D') board[i] = 5;
        else if (c == 'H') board[i] = 6;
    }
    
    printf("Board: %s\n", board_str.c_str());
    
    // 分析最佳3x3目标
    find_best_3x3_target(board);
    
    if (g_best_target.target_score >= 0) {
        printf("Best 3x3 target: Orb %d at (%d,%d), Score: %d\n", 
               g_best_target.orb_type, g_best_target.top_row, g_best_target.top_col, 
               g_best_target.target_score);
        
        printf("Target area layout:\n");
        for (int i = 0; i < 3; i++) {
            printf("  ");
            for (int j = 0; j < 3; j++) {
                int pos = (g_best_target.top_row + i) * 6 + (g_best_target.top_col + j);
                char orb_char = board[pos] == 0 ? '.' : ('A' + board[pos] - 1);
                if (board[pos] == g_best_target.orb_type) {
                    printf("[%c]", orb_char);
                } else {
                    printf(" %c ", orb_char);
                }
            }
            printf("\n");
        }
        
        // 测试评估函数
        state test_state;
        heuristic_3x3_evaluate(board, test_state);
        printf("Heuristic evaluation: Score=%d, Goal=%s\n", 
               test_state.score, test_state.goal ? "YES" : "NO");
        
    } else {
        printf("No viable 3x3 target found (need 9+ orbs of same color)\n");
    }
    
    return 0;
}