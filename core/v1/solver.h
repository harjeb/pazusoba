/**
 * solver.h
 * by Yiheng Quan
 */

#ifndef PAD_SOLVER_H
#define PAD_SOLVER_H

#include <string>
#include <vector>
#include <set>
#include <optional>
#include "pad.h"
#include "route.h"
#include "board.h"
#include "state.h"

// Forward declaration
struct SolverConfig;
class Profile;

// 十字目标结构
struct CrossTarget {
    int centerX, centerY;           // 十字中心位置
    pad::orbs targetColor;          // 目标颜色
    int estimatedSteps;             // 预估需要的移动步数
    int expectedCombos;             // 预期能形成的combo数量
    double comboEfficiency;         // combo效率 = expectedCombos / estimatedSteps
    std::vector<std::pair<int, int>> requiredPositions; // 需要填充的位置
    
    CrossTarget(int x, int y, pad::orbs color, int steps, int combos = 1) 
        : centerX(x), centerY(y), targetColor(color), estimatedSteps(steps), expectedCombos(combos) {
        comboEfficiency = (steps > 0) ? (double)combos / steps : 0.0;
    }
};

// 9宫格目标结构
struct NineTarget {
    int centerX, centerY;           // 9宫格中心位置
    pad::orbs targetColor;          // 目标颜色
    int estimatedSteps;             // 预估需要的移动步数
    int expectedCombos;             // 预期能形成的combo数量
    double comboEfficiency;         // combo效率 = expectedCombos / estimatedSteps
    std::vector<std::pair<int, int>> requiredPositions; // 需要填充的位置
    
    NineTarget(int x, int y, pad::orbs color, int steps, int combos = 1) 
        : centerX(x), centerY(y), targetColor(color), estimatedSteps(steps), expectedCombos(combos) {
        comboEfficiency = (steps > 0) ? (double)combos / steps : 0.0;
    }
};

// 珠子移动计划
struct OrbMoveplan {
    int fromX, fromY;               // 源位置
    int toX, toY;                   // 目标位置
    pad::orbs orbType;              // 珠子类型
    int priority;                   // 移动优先级
    
    OrbMoveplan(int fx, int fy, int tx, int ty, pad::orbs orb, int pri = 0)
        : fromX(fx), fromY(fy), toX(tx), toY(ty), orbType(orb), priority(pri) {}
};

class PSolver
{
    int row = 0;
    int column = 0;
    int minErase = 3;
    int steps = 25;
    int size = 1000;
    bool debug = true;
    
    // Display configuration
    bool showFinalBoard = true;
    bool showRoutePath = true;
    bool showScore = true;
    bool showBoardTransform = true;
    bool verbose = false;

    /// Read board from filePath, return the board
    Board readBoard(const std::string &filePath);
    
    /// Create default profiles
    std::vector<Profile*> createProfiles() const;
    
    /// Create profiles based on configuration
    std::vector<Profile*> createProfiles(const SolverConfig &config) const;
    
    /// 十字目标导向算法相关方法
    std::vector<CrossTarget> findPossibleCrosses(const PBoard& board, pad::orbs targetColor, bool verbose = false) const;
    bool canFormCross(const PBoard& board, int centerX, int centerY, pad::orbs targetColor) const;
    int estimateCrossSteps(const PBoard& board, int centerX, int centerY, pad::orbs targetColor) const;
    int estimateTotalCombos(const PBoard& board, int centerX, int centerY, pad::orbs targetColor, bool verbose = false) const;
    std::vector<OrbMoveplan> planCrossMoves(const PBoard& board, const CrossTarget& target, bool verbose = false) const;
    std::vector<Route> solveCrossTargeted(const SolverConfig& config) const;
    
    /// 9宫格目标导向算法相关方法
    std::vector<NineTarget> findPossibleNineGrids(const PBoard& board, pad::orbs targetColor, bool verbose = false) const;
    bool canFormNineGrid(const PBoard& board, int centerX, int centerY, pad::orbs targetColor) const;
    int estimateNineGridSteps(const PBoard& board, int centerX, int centerY, pad::orbs targetColor) const;
    int estimateNineGridCombos(const PBoard& board, int centerX, int centerY, pad::orbs targetColor, bool verbose = false) const;
    std::vector<OrbMoveplan> planNineGridMoves(const PBoard& board, const NineTarget& target, bool verbose = false) const;
    std::vector<Route> solveNineGridTargeted(const SolverConfig& config) const;
    
    /// 9宫格路径生成辅助方法
    std::vector<Route> generateNineGridRoutes(const NineTarget& target, const std::vector<OrbMoveplan>& movePlan, const SolverConfig& config) const;
    PState* buildOptimalNineGridState(const NineTarget& target, const std::vector<OrbMoveplan>& movePlan) const;
    std::vector<std::pair<int, int>> calculateOptimalMoveSequence(const std::vector<OrbMoveplan>& movePlan) const;
    bool validateNineGridFormation(const Route& route, pad::orbs targetColor, bool verbose = false) const;
    
    /// 分布式聚集算法专用的9宫格求解方法
    std::vector<Route> solveNineGridDistributed(const SolverConfig& config) const;
    
    /// 分布式算法：第一阶段 - 珠子聚集
    std::vector<std::pair<int, int>> phaseOneGatherOrbs(const NineTarget& target, bool verbose = false) const;
    
    /// 分布式算法：第二阶段 - 精确排列
    std::vector<std::pair<int, int>> phaseTwoArrangeGrid(const PBoard& clusteredBoard, const NineTarget& target, const std::pair<int, int>& phase1EndPos, bool verbose = false) const;
    
    /// 分布式算法：珠子聚集策略
    std::vector<std::pair<int, int>> moveOrbToCluster(const PBoard& board, int fromX, int fromY, const NineTarget& target) const;
    
    /// 创建精确的移动序列
    std::vector<std::pair<int, int>> createPreciseMoveSequence(const std::pair<int, int>& from, const std::pair<int, int>& to, const PBoard& board) const;
    
    /// 分布式路径转换为Route的辅助方法
    std::optional<Route> convertDistributedPathToRoute(const std::vector<std::pair<int, int>>& distributedPath, 
                                                      const NineTarget& target, 
                                                      const SolverConfig& config) const;
    
    /// Helper method to get orb from PBoard
    pad::orbs getOrbAt(const PBoard& pboard, int x, int y) const;

public:
    /// This is the original board
    PBoard board;

    PSolver(const std::string &filePath, int minErase, int steps, int size);
    PSolver(const SolverConfig &config);

    /// Solve the current board
    std::vector<Route> solve(bool resetProfiles = true);
    
    /// Solve with custom profile configuration
    std::vector<Route> solve(const SolverConfig &config);

    /// Read a board from a string
    void setBoardFrom(const std::string &board);

    /// A random board that is row x column
    void setRandomBoard(int row, int column);

    /// Update beam size, mainly for Qt
    void setBeamSize(int size);

    /// Update step limit
    void setStepLimit(int step);
};

#endif
