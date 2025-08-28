/**
 * solver_config.h
 * Extended configuration for pazusoba solver
 * by Yiheng Quan
 */

#ifndef SOLVER_CONFIG_H
#define SOLVER_CONFIG_H

#include <vector>
#include <string>
#include <set>
#include "pad.h"

struct SolverConfig
{
    // Basic parameters
    std::string filePath = "RHGHDRGGBBGGDBLLHBGGGRLHGHDGLG";
    int minErase = 3;
    int maxStep = 30;
    int maxSize = 20000;
    
    // Color priority settings
    std::vector<pad::orbs> priorityColors;
    
    // Shape priority settings
    bool enablePlusProfile = false;      // 十字类型
    bool enableNineProfile = false;      // 9宫格类型
    bool enableLProfile = false;         // L型
    bool enableTwoWayProfile = false;    // 2U型
    bool enableOneRowProfile = false;    // 单行型
    bool enableOneColumnProfile = false; // 单列型
    std::vector<pad::orbs> plusColors;   // 十字优先颜色
    std::vector<pad::orbs> nineColors;   // 9宫格优先颜色
    std::vector<pad::orbs> lColors;      // L型优先颜色
    std::vector<pad::orbs> twoWayColors; // 2U型优先颜色
    std::vector<pad::orbs> oneRowColors; // 单行优先颜色
    std::vector<pad::orbs> oneColumnColors; // 单列优先颜色
    
    // Constraint mode settings (强制约束模式)
    bool enablePlusConstraint = false;   // 十字强制约束：有足够珠子就必须组成十字
    bool enableNineConstraint = false;   // 9宫格强制约束：有足够珠子就必须组成9宫格
    
    // Movement settings
    bool enableDiagonalMovement = true;  // 斜向移动开关
    
    // Display settings
    bool showFinalBoard = true;          // 显示最终棋盘
    bool showRoutePath = true;           // 显示路径
    bool showScore = true;               // 显示分数
    bool showBoardTransform = true;      // 显示棋盘变化 (初始 -> 最终)
    bool verbose = false;                // 详细输出
    
    // Helper functions
    static pad::orbs parseColor(const std::string& colorStr);
    static std::string colorToString(pad::orbs color);
    static void printUsage();
    
    // Parse color list from string (e.g., "RGB" -> {fire, water, wood})
    static std::vector<pad::orbs> parseColorList(const std::string& colorStr);
};

#endif