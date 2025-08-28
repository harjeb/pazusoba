#include "core/v1/solver.h"
#include "core/v1/solver_config.h"
#include <iostream>

int main() {
    std::cout << "Testing Guaranteed 3x3 Algorithm..." << std::endl;
    
    // 测试棋盘: DRGDGGRGHRGLLBRGHHRGBHGGGRBGGL
    std::string testBoard = "DRGDGGRGHRGLLBRGHHRGBHGGGRBGGL";
    
    // 创建solver配置
    SolverConfig config;
    config.filePath = testBoard;
    config.minErase = 3;
    config.maxStep = 20;
    config.maxSize = 1000;
    config.verbose = true;
    config.enableNineConstraint = true;
    config.nineColors.push_back(pad::wood); // G = Wood
    
    // 创建solver实例
    PSolver solver(config);
    
    std::cout << "Initial Board:" << std::endl;
    std::cout << "DRGDGG" << std::endl;
    std::cout << "RGHRGL" << std::endl;
    std::cout << "LBRGHH" << std::endl;
    std::cout << "RGBHGG" << std::endl;
    std::cout << "GRBGGL" << std::endl;
    std::cout << std::endl;
    
    // 测试保证算法
    std::cout << "Calling guaranteed algorithm..." << std::endl;
    auto routes = solver.solveNineGridGuaranteed(config);
    
    if (!routes.empty()) {
        std::cout << "SUCCESS! Guaranteed algorithm found " << routes.size() << " routes!" << std::endl;
        std::cout << "First route:" << std::endl;
        routes[0].printRoute();
        std::cout << std::endl;
        std::cout << "Final board:" << std::endl;
        std::cout << routes[0].getFinalBoardStringMultiLine() << std::endl;
    } else {
        std::cout << "Algorithm did not find any valid routes." << std::endl;
    }
    
    return 0;
}