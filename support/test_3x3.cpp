#include <pazusoba/core.h>
#include <iostream>

void test_3x3_square() {
    printf("=== Testing 3x3 Square Detection ===\n");
    
    pazusoba::solver solver;
    
    // Test case 1: Simple 3x3 square
    printf("\nTest 1: Simple 3x3 square\n");
    solver.set_board("RRRBBBGGGRRRBBBGGGRRRBBBGGGRR");
    auto board = solver.board();
    solver.print_board(board);
    
    pazusoba::combo_list combos;
    solver.erase_combo(board, combos);
    
    printf("Found %d combos:\n", (int)combos.size());
    for (const auto& c : combos) {
        printf("  Orb %d with %d locations: ", c.info, (int)c.loc.size());
        for (int loc : c.loc) {
            printf("%d ", loc);
        }
        printf("\n");
        
        if (c.loc.size() == 9) {
            bool is_square = solver.is_3x3_square(c.loc, solver.column());
            printf("  Is 3x3 square: %s\n", is_square ? "YES" : "NO");
        }
    }
    
    // Test case 2: Mixed board with one 3x3 square
    printf("\nTest 2: Mixed board with one 3x3 square\n");
    solver.set_board("RRRRRRRRRBBGGGGGGGGGGRRRBBBGGG");
    board = solver.board();
    solver.print_board(board);
    
    combos.clear();
    solver.erase_combo(board, combos);
    
    printf("Found %d combos:\n", (int)combos.size());
    for (const auto& c : combos) {
        printf("  Orb %d with %d locations: ", c.info, (int)c.loc.size());
        for (int loc : c.loc) {
            printf("%d ", loc);
        }
        printf("\n");
        
        if (c.loc.size() == 9) {
            bool is_square = solver.is_3x3_square(c.loc, solver.column());
            printf("  Is 3x3 square: %s\n", is_square ? "YES" : "NO");
        }
    }
    
    printf("\n=== Test completed ===\n");
}

int main() {
    test_3x3_square();
    return 0;
}