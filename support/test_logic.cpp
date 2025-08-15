#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

// Simple test for 3x3 square detection logic
bool is_3x3_square(const std::unordered_set<int>& locations, int column, int row) {
    if (locations.size() != 9) return false;
    
    // Find the minimum row and column to determine top-left corner
    int min_row = row, min_col = column;
    for (int loc : locations) {
        int r = loc / column;
        int c = loc % column;
        min_row = std::min(min_row, r);
        min_col = std::min(min_col, c);
    }
    
    // Check if all 9 positions in the 3x3 square starting from (min_row, min_col) are present
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int expected_loc = (min_row + i) * column + (min_col + j);
            if (locations.find(expected_loc) == locations.end()) {
                return false;
            }
        }
    }
    
    return true;
}

void test_3x3_logic() {
    printf("=== Testing 3x3 Square Logic ===\n");
    
    // Test case 1: Valid 3x3 square
    std::unordered_set<int> square1 = {0, 1, 2, 6, 7, 8, 12, 13, 14}; // Top-left 3x3
    bool result1 = is_3x3_square(square1, 6, 5);
    printf("Test 1 - Valid 3x3 square: %s\n", result1 ? "PASS" : "FAIL");
    
    // Test case 2: Invalid (not 9 locations)
    std::unordered_set<int> square2 = {0, 1, 2, 6, 7, 8, 12, 13};
    bool result2 = is_3x3_square(square2, 6, 5);
    printf("Test 2 - Invalid size: %s\n", !result2 ? "PASS" : "FAIL");
    
    // Test case 3: Invalid (not a square)
    std::unordered_set<int> square3 = {0, 1, 2, 3, 6, 7, 8, 9, 12};
    bool result3 = is_3x3_square(square3, 6, 5);
    printf("Test 3 - Not a square: %s\n", !result3 ? "PASS" : "FAIL");
    
    // Test case 4: Valid 3x3 square at different position
    std::unordered_set<int> square4 = {15, 16, 17, 21, 22, 23, 27, 28, 29}; // Bottom-right 3x3
    bool result4 = is_3x3_square(square4, 6, 5);
    printf("Test 4 - Valid 3x3 square (different position): %s\n", result4 ? "PASS" : "FAIL");
    
    printf("\n=== Logic Test completed ===\n");
}

int main() {
    test_3x3_logic();
    return 0;
}