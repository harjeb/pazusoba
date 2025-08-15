// pazusoba_force3x3.cpp
// Modified version that ONLY cares about 3x3 squares
// Ignores all other scoring factors

#include <pazusoba/core.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <queue>
#include <thread>
#include <vector>

namespace pazusoba {

// Override the evaluate function to ONLY care about 3x3 squares
void force_3x3_evaluate(game_board& board, state& new_state, solver& s) {
    short int score = 0;
    
    // Only care about finding 3x3 squares, ignore everything else
    combo_list list;
    game_board copy = board;
    
    // Just run one erase cycle to find current combos
    s.erase_combo(copy, list);
    
    int goal = 0;
    bool found_3x3 = false;
    
    // Check each combo to see if it forms a 3x3 square
    for (const auto& c : list) {
        int size = c.loc.size();
        if (size >= 9) {
            // Check if it forms a 3x3 square
            if (s.is_3x3_square(c.loc, s.column())) {
                score += 10000;  // Massive score for 3x3
                goal = 1;
                found_3x3 = true;
                printf("Found 3x3 square with %d orbs of type %d!\n", size, c.info);
            }
        } else if (size >= 6) {
            // Give some points for large clusters that could become 3x3
            score += size * 10;
        }
    }
    
    // If we found a 3x3, that's all we care about
    if (found_3x3) {
        new_state.score = score;
        new_state.goal = true;
        new_state.combo = list.size();
        return;
    }
    
    // If no 3x3 found, evaluate potential for forming one
    // Count orbs of each type
    int orb_count[ORB_COUNT] = {0};
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board[i] > 0) {
            orb_count[board[i]]++;
        }
    }
    
    // Find colors that have potential for 3x3 (9+ orbs)
    for (int orb_type = 1; orb_type < ORB_COUNT; orb_type++) {
        if (orb_count[orb_type] >= 9) {
            // This color can potentially form 3x3
            // Check how close we are to forming one
            score += analyze_3x3_potential(board, orb_type, s);
        }
    }
    
    new_state.score = score;
    new_state.goal = goal > 0;
    new_state.combo = list.size();
}

// Analyze how close a specific orb type is to forming a 3x3
int analyze_3x3_potential(game_board& board, int orb_type, solver& s) {
    int potential_score = 0;
    int rows = s.row();
    int cols = s.column();
    
    // Check all possible 3x3 positions
    for (int top_row = 0; top_row <= rows - 3; top_row++) {
        for (int top_col = 0; top_col <= cols - 3; top_col++) {
            int matching_orbs = 0;
            
            // Count matching orbs in this 3x3 area
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int pos = (top_row + i) * cols + (top_col + j);
                    if (board[pos] == orb_type) {
                        matching_orbs++;
                    }
                }
            }
            
            // Score based on how many orbs already match
            if (matching_orbs >= 6) {
                potential_score += 500 + matching_orbs * 100; // Very close!
            } else if (matching_orbs >= 4) {
                potential_score += 200 + matching_orbs * 50;  // Getting there
            } else if (matching_orbs >= 2) {
                potential_score += 50 + matching_orbs * 10;   // Some potential
            }
        }
    }
    
    return potential_score;
}

class force_3x3_solver : public solver {
public:
    // Override the evaluate method
    void evaluate(game_board& board, state& new_state) override {
        force_3x3_evaluate(board, new_state, *this);
    }
};

}

int main(int argc, char* argv[]) {
    // Use our custom solver
    auto force_solver = pazusoba::force_3x3_solver();
    force_solver.parse_args(argc, argv);

    // Set up profile for 3x3 squares only
    pazusoba::profile profiles[1];
    profiles[0].name = pazusoba::shape_square;
    profiles[0].stop_threshold = 300;  // Allow more exploration
    
    // Enable all orb types
    for (int i = 0; i < pazusoba::ORB_COUNT; i++) {
        profiles[0].orbs[i] = true;
    }
    
    force_solver.set_profiles(profiles, 1);

    pazusoba::Timer timer("force 3x3 search");
    auto state = force_solver.adventure();
    force_solver.print_state(state);
    return 0;
}