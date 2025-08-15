#include <iostream>
#include <pazusoba/core.h>

int main() {
    // Test diagonal movement functionality
    std::cout << "Testing diagonal movement functionality..." << std::endl;
    
    // Check if diagonal directions are defined
    std::cout << "Direction count: " << pazusoba::DIRECTION_COUNT << std::endl;
    
    // Test directions
    std::cout << "up = " << pazusoba::up << std::endl;
    std::cout << "down = " << pazusoba::down << std::endl;
    std::cout << "left = " << pazusoba::left << std::endl;
    std::cout << "right = " << pazusoba::right << std::endl;
    std::cout << "up_left = " << pazusoba::up_left << std::endl;
    std::cout << "up_right = " << pazusoba::up_right << std::endl;
    std::cout << "down_left = " << pazusoba::down_left << std::endl;
    std::cout << "down_right = " << pazusoba::down_right << std::endl;
    
    // Create a test solver
    auto solver = pazusoba::solver();
    
    // Set a random test board
    std::string test_board = "RRRBBBGGGLLLDDDHHHRRRBBBLLLGGG";
    solver.set_board(test_board, 5, 6);
    
    // Test adventure solving with diagonal moves
    std::cout << "Running solver with diagonal movement support..." << std::endl;
    auto result = solver.adventure();
    
    std::cout << "Result score: " << result.score << std::endl;
    std::cout << "Result combo: " << result.combo << std::endl;
    std::cout << "Steps taken: " << result.step << std::endl;
    
    return 0;
}