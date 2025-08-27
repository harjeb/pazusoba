// 简单测试文件来验证修复逻辑
#include "core/v1/solver_config.h"
#include <iostream>

int main() {
    SolverConfig config;
    
    // 测试enablePlusConstraint成员是否存在
    config.enablePlusConstraint = true;
    config.enableNineConstraint = true;
    
    std::cout << "enablePlusConstraint: " << config.enablePlusConstraint << std::endl;
    std::cout << "enableNineConstraint: " << config.enableNineConstraint << std::endl;
    
    return 0;
}