#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

// Mock the pazusoba namespace for testing
namespace pazusoba {
    typedef unsigned char orb;
    typedef std::array<orb, 42> game_board;
    typedef std::array<orb, 42> visit_board;
    
    struct combo {
        orb info;
        std::unordered_set<int> loc;
        combo(const orb& o) : info(o) {}
    };
    typedef std::vector<combo> combo_list;
    
    // Constants
    const int ROW = 5;
    const int COLUMN = 6;
    const int BOARD_SIZE = 30;
    #define INDEX_OF(x, y) (x * COLUMN + y)
    
    // Test implementation
    bool is_3x3_square(const std::unordered_set<int>& locations) {
        if (locations.size() != 9) return false;
        
        // Find the minimum row and column to determine top-left corner
        int min_row = ROW, min_col = COLUMN;
        for (int loc : locations) {
            int row = loc / COLUMN;
            int col = loc % COLUMN;
            min_row = std::min(min_row, row);
            min_col = std::min(min_col, col);
        }
        
        // Check if all 9 positions in the 3x3 square are present
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
    
    void check_3x3_squares(game_board& board, combo_list& list, visit_board& visited_location) {
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
    }
    
    void print_board(const game_board& board) {
        printf("Board:\n");
        for (int i = 0; i < ROW; i++) {
            for (int j = 0; j < COLUMN; j++) {
                int index = INDEX_OF(i, j);
                printf("%c ", board[index] == 0 ? '.' : 'A' + board[index] - 1);
            }
            printf("\n");
        }
        printf("\n");
    }
    
    void set_board_from_string(game_board& board, const std::string& board_string) {
        for (int i = 0; i < BOARD_SIZE && i < board_string.size(); i++) {
            char c = board_string[i];
            if (c >= 'A' && c <= 'Z') {
                board[i] = c - 'A' + 1;
            } else {
                board[i] = 0;
            }
        }
    }
}

void test_3x3_detection() {
    printf("=== Testing 3x3 Square Detection ===\n");
    
    // Test case 1: Simple 3x3 square
    printf("\nTest 1: Simple 3x3 square\n");
    pazusoba::game_board board = {0};
    pazusoba::set_board_from_string(board, "AAABBBCCCDDDAAABBBCCCDDDAAABBB");
    pazusoba::print_board(board);
    
    pazusoba::combo_list combos;
    pazusoba::visit_board visited = {0};
    pazusoba::check_3x3_squares(board, combos, visited);
    
    printf("Found %d combos:\n", (int)combos.size());
    for (const auto& c : combos) {
        printf("  Combo: Orb %c with %d locations\n", 'A' + c.info - 1, (int)c.loc.size());
        printf("  Locations: ");
        for (int loc : c.loc) {
            printf("%d ", loc);
        }
        printf("\n");
        
        bool is_square = pazusoba::is_3x3_square(c.loc);
        printf("  Is 3x3 square: %s\n", is_square ? "YES" : "NO");
    }
    
    // Test case 2: Multiple 3x3 squares
    printf("\nTest 2: Multiple 3x3 squares\n");
    board = {0};
    pazusoba::set_board_from_string(board, "AAABBBCCCDDDAAABBBCCCDDDAAABBB");
    pazusoba::print_board(board);
    
    combos.clear();
    visited = {0};
    pazusoba::check_3x3_squares(board, combos, visited);
    
    printf("Found %d combos:\n", (int)combos.size());
    for (const auto& c : combos) {
        printf("  Combo: Orb %c with %d locations\n", 'A' + c.info - 1, (int)c.loc.size());
        bool is_square = pazusoba::is_3x3_square(c.loc);
        printf("  Is 3x3 square: %s\n", is_square ? "YES" : "NO");
    }
    
    // Test case 3: Mixed board with one 3x3 square
    printf("\nTest 3: Mixed board with one 3x3 square\n");
    board = {0};
    pazusoba::set_board_from_string(board, "AAABBBCCCDDDEEEFFFGGGAAABBB");
    pazusoba::print_board(board);
    
    combos.clear();
    visited = {0};
    pazusoba::check_3x3_squares(board, combos, visited);
    
    printf("Found %d combos:\n", (int)combos.size());
    for (const auto& c : combos) {
        printf("  Combo: Orb %c with %d locations\n", 'A' + c.info - 1, (int)c.loc.size());
        bool is_square = pazusoba::is_3x3_square(c.loc);
        printf("  Is 3x3 square: %s\n", is_square ? "YES" : "NO");
    }
    
    // Test case 4: No 3x3 squares
    printf("\nTest 4: No 3x3 squares\n");
    board = {0};
    pazusoba::set_board_from_string(board, "ABABABABABABABABABABABABABABAB");
    pazusoba::print_board(board);
    
    combos.clear();
    visited = {0};
    pazusoba::check_3x3_squares(board, combos, visited);
    
    printf("Found %d combos:\n", (int)combos.size());
    for (const auto& c : combos) {
        printf("  Combo: Orb %c with %d locations\n", 'A' + c.info - 1, (int)c.loc.size());
    }
    
    printf("\n=== Test completed ===\n");
}

void test_is_3x3_square_function() {
    printf("\n=== Testing is_3x3_square function ===\n");
    
    // Test case 1: Valid 3x3 square
    std::unordered_set<int> square1 = {0, 1, 2, 6, 7, 8, 12, 13, 14};
    bool result1 = pazusoba::is_3x3_square(square1);
    printf("Test 1 - Valid 3x3 square: %s\n", result1 ? "PASS" : "FAIL");
    
    // Test case 2: Invalid (not 9 locations)
    std::unordered_set<int> square2 = {0, 1, 2, 6, 7, 8, 12, 13};
    bool result2 = pazusoba::is_3x3_square(square2);
    printf("Test 2 - Invalid size: %s\n", !result2 ? "PASS" : "FAIL");
    
    // Test case 3: Invalid (not a square)
    std::unordered_set<int> square3 = {0, 1, 2, 3, 6, 7, 8, 9, 12};
    bool result3 = pazusoba::is_3x3_square(square3);
    printf("Test 3 - Not a square: %s\n", !result3 ? "PASS" : "FAIL");
    
    // Test case 4: Valid 3x3 square at different position
    std::unordered_set<int> square4 = {15, 16, 17, 21, 22, 23, 27, 28, 29};
    bool result4 = pazusoba::is_3x3_square(square4);
    printf("Test 4 - Valid 3x3 square (different position): %s\n", result4 ? "PASS" : "FAIL");
    
    // Test case 5: Invalid (missing corner)
    std::unordered_set<int> square5 = {0, 1, 2, 6, 7, 8, 12, 13, 18}; // 18 instead of 14
    bool result5 = pazusoba::is_3x3_square(square5);
    printf("Test 5 - Missing corner: %s\n", !result5 ? "PASS" : "FAIL");
    
    printf("=== Function test completed ===\n");
}

int main() {
    test_is_3x3_square_function();
    test_3x3_detection();
    return 0;
}