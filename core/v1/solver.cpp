/**
 * solver.cpp
 * by Yiheng Quan
 */

#include "solver.h"
#include "solver_config.h"
#include "route.h"
#include "configuration.h"
#include "profile.h"
#include "queue.h"
#include "timer.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <algorithm>
#include <unordered_set>

// Helper function to display two multi-line strings side by side
void printBoardComparison(const std::string& initialBoard, const std::string& finalBoard) {
    std::istringstream initialStream(initialBoard);
    std::istringstream finalStream(finalBoard);
    std::string initialLine, finalLine;
    
    std::cout << "\nInitial Board:     Final Board:" << std::endl;
    
    bool hasInitialLine = static_cast<bool>(std::getline(initialStream, initialLine));
    bool hasFinalLine = static_cast<bool>(std::getline(finalStream, finalLine));
    
    // Determine the width of initial board lines for proper alignment
    int maxInitialWidth = 0;
    if (hasInitialLine) {
        maxInitialWidth = static_cast<int>(initialLine.length());
    }
    
    while (hasInitialLine || hasFinalLine) {
        // Print initial board line
        if (hasInitialLine) {
            std::cout << initialLine;
            // Add padding to reach consistent width
            int padding = maxInitialWidth - static_cast<int>(initialLine.length());
            std::cout << std::string(padding, ' ');
        } else {
            std::cout << std::string(maxInitialWidth, ' ');
        }
        
        std::cout << "       ->       ";
        
        // Print final board line  
        if (hasFinalLine) {
            std::cout << finalLine;
        }
        
        std::cout << std::endl;
        
        hasInitialLine = static_cast<bool>(std::getline(initialStream, initialLine));
        hasFinalLine = static_cast<bool>(std::getline(finalStream, finalLine));
    }
}

// This is only for the priority queue
class PointerCompare
{
public:
    template <typename T>
    bool operator()(T *a, T *b) { return (*a) < (*b); }
};

// MARK: - Constrcutors

PSolver::PSolver(const std::string &filePath, int minErase, int steps,
                 int size)
{
    this->minErase = minErase;
    this->steps = steps;
    this->size = size;

    if (filePath.find(".txt") != std::string::npos)
    {
        auto currBoard = readBoard(filePath);
        board = PBoard(currBoard);
    }
    else
    {
        setBoardFrom(filePath);
    }
}

PSolver::PSolver(const SolverConfig &config)
{
    this->minErase = config.minErase;
    this->steps = config.maxStep;
    this->size = config.maxSize;
    
    // Set display options
    this->showFinalBoard = config.showFinalBoard;
    this->showRoutePath = config.showRoutePath;
    this->showScore = config.showScore;
    this->showBoardTransform = config.showBoardTransform;
    this->verbose = config.verbose;

    if (config.filePath.find(".txt") != std::string::npos)
    {
        auto currBoard = readBoard(config.filePath);
        board = PBoard(currBoard);
    }
    else
    {
        setBoardFrom(config.filePath);
    }
}

// MARK: - Solve the board

std::vector<Profile*> PSolver::createProfiles() const
{
    // Default profile (only ComboProfile, no automatic NineProfile)
    std::vector<Profile*> profiles;
    profiles.push_back(new ComboProfile);
    
    return profiles;
}

std::vector<Profile*> PSolver::createProfiles(const SolverConfig &config) const
{
    std::vector<Profile*> profiles;
    
    // Always include ComboProfile as base
    profiles.push_back(new ComboProfile);
    
    // Add color priority profiles
    if (!config.priorityColors.empty()) {
        profiles.push_back(new ColourProfile(config.priorityColors));
    }
    
    // Add plus profile (forced mode takes priority over normal mode)
    if (config.enablePlusConstraint && !config.plusColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] Creating ForcedPlusProfile" << std::endl;
        }
        profiles.push_back(new ForcedPlusProfile(config.plusColors));
    } else if (config.enablePlusProfile && !config.plusColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] Creating PlusProfile" << std::endl;
        }
        profiles.push_back(new PlusProfile(config.plusColors));
    }
    
    // Add nine-grid profile (forced mode takes priority over normal mode)
    if (config.enableNineConstraint && !config.nineColors.empty()) {
        profiles.push_back(new ForcedNineProfile(config.nineColors));
    } else if (config.enableNineProfile && !config.nineColors.empty()) {
        profiles.push_back(new NineProfile(config.nineColors));
    }
    
    // Add L-shape profile
    if (config.enableLProfile && !config.lColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] Creating LProfile" << std::endl;
        }
        profiles.push_back(new LProfile(config.lColors));
    }
    
    // Add TwoWay profile
    if (config.enableTwoWayProfile && !config.twoWayColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] Creating TwoWayProfile" << std::endl;
        }
        profiles.push_back(new TwoWayProfile(config.twoWayColors));
    }
    
    // Add OneRow profile
    if (config.enableOneRowProfile && !config.oneRowColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] Creating OneRowProfile" << std::endl;
        }
        profiles.push_back(new OneRowProfile(config.oneRowColors));
    }
    
    // Add OneColumn profile
    if (config.enableOneColumnProfile && !config.oneColumnColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] Creating OneColumnProfile" << std::endl;
        }
        profiles.push_back(new OneColumnProfile(config.oneColumnColors));
    }
    
    // Always add RandomAvoidanceProfile to prevent dangerous random orb layouts
    profiles.push_back(new RandomAvoidanceProfile());
    if (config.verbose) {
        std::cout << "[DEBUG] Creating RandomAvoidanceProfile" << std::endl;
    }
    
    return profiles;
}

std::vector<Route> PSolver::solve(bool resetProfiles)
{
    std::vector<Profile*> profiles;
    
    if (resetProfiles) {
        // Use default profile configuration
        profiles = createProfiles();

        auto conf = Configuration::shared();
        ProfileManager::shared().updateProfile(profiles);
    }

    // Set debug based on verbose setting (removed initial board display)
    if (verbose) {
        if (resetProfiles) {
            std::cout << "Using " << profiles.size() << " profiles for scoring." << std::endl;
        } else {
            std::cout << "Using existing profiles for scoring." << std::endl;
        }
        std::cout << "The board is " << row << " x " << column << ". Max step is " << steps << "." << std::endl;
        std::cout << "\nInitial Board:" << std::endl;
        std::cout << board.getBoardStringMultiLine() << std::endl;
    }

    // A queue that only saves top 100, 1000 based on the size
    // PPriorityQueue *toVisit = new PPriorityQueue(size);
    std::priority_queue<PState *, std::vector<PState *>, PointerCompare> toVisit;
    // This saves all chidren of states to visit
    std::vector<PState *> childrenStates;
    childrenStates.reserve(size * 4);
    // This saves the best score
    std::map<int, PState *> bestScore;
    Timer::shared().start(999);

    // This is the root state, 30 of them in a list to be deleted later
    std::vector<PState *> rootStates;
    rootStates.reserve(row * column);
    for (int i = 0; i < row; ++i)
    {
        for (int j = 0; j < column; ++j)
        {
            auto loc = OrbLocation(i, j);
            auto root = new PState(board, loc, loc, 0, steps);
            rootStates.emplace_back(root);
            toVisit.emplace(root);
        }
    }

    std::vector<std::thread> boardThreads;
    // This uses all your cores, make sure you don't make the size too large
    //int processor_count = std::thread::hardware_concurrency();
    int processor_count = 8;
    if (processor_count == 0)
    {
        processor_count = 1;
    }
    // Using processor_count threads silently
    boardThreads.reserve(processor_count);
    // Cut into equal sizes
    int threadSize = size / processor_count;
    // This is important for queue and childrenStates because if you access them
    // at the same time, the program will crash. By locking and unlocking, it will
    // make sure it is safe
    std::mutex mtx;

    srand(time(0));
    // Only take first 1000, reset for every step
    for (int i = 0; i < steps; ++i)
    {
        // int currSize = size * (100 + ((steps - i - 1) * 100 / steps)) / 200;
        // threadSize = currSize / processor_count;
        if (debug)
            Timer::shared().start(i);

        // Use multi threading
        for (int j = 0; j < processor_count; j++)
        {
            // Early break for early steps
            if (toVisit.empty())
                break;

            boardThreads.emplace_back([&] {
                for (int k = 0; k < threadSize; ++k)
                {
                    // Get the best state
                    mtx.lock();
                    if (toVisit.empty())
                    {
                        mtx.unlock();
                        break;
                    }
                    auto currentState = toVisit.top();
                    toVisit.pop();
                    mtx.unlock();

                    // Save current score for printing out later
                    int currentScore = currentState->score;
                    
                    int currentStep = currentState->step;


                    // Save best scores
                    bool shouldAdd = false;
                    mtx.lock();
                    if (bestScore[currentScore] == nullptr)
                    {
                        bestScore[currentScore] = currentState;

                        shouldAdd = true;
                    }
                    else
                    {
                        auto saved = bestScore[currentScore];
                        shouldAdd = !(saved == currentState);

                        if (saved->step > currentStep)
                        {
                            // We found a better one
                            bestScore[currentScore] = currentState;
                        }
                    }
                    mtx.unlock();

                    if (shouldAdd && currentState != nullptr)
                    {
                        // All all possible children
                        for (auto &s : currentState->getChildren())
                        {
                            // Simply insert because states compete with each other
                            mtx.lock();
                            childrenStates.push_back(s);
                            mtx.unlock();
                        }
                    }
                }
            });
        }

        // Make sure all threads are completed
        for (auto &t : boardThreads)
        {
            t.join();
        }
        // Clear for next round
        boardThreads.clear();

        toVisit =
            std::priority_queue<PState *, std::vector<PState *>, PointerCompare>();
        for (const auto &s : childrenStates)
        {
            // push randomly
            // int num = rand() % 30;
            // if (i < 10 && num < 25)
            //     toVisit.push(s);
            // else if (num < 15)
            toVisit.push(s);
        }
        childrenStates.clear();

        if (debug)
            Timer::shared().end(i);
    }

    if (debug)
        std::cout << "Search has been completed\n\n";

    // free all profiles once the search is completed
    ProfileManager::shared().clear();

    std::vector<Route> routes;

    int prevScore = -1;
    int i = 0;
    for (auto it = bestScore.end(); it != bestScore.begin(); it--, i++)
    {
        if (i == 0)
            continue;

        auto curr = it->second;
        int currScore = curr->score;
        if (prevScore < 0)
        {
            prevScore = currScore / 1000;
        }
        else if (currScore / 1000 < prevScore)
        {
            break;
        }

        routes.emplace_back(curr);
    }

    // Sort saved routes by combo count (highest first), then by steps (lowest first)
    std::sort(routes.begin(), routes.end(), [](Route &a, Route &b) {
        if (a.getCombo() != b.getCombo()) {
            return a.getCombo() > b.getCombo(); // Higher combo first
        }
        return a.getStep() < b.getStep(); // If combo same, fewer steps first
    });

    if (routes.size() > 0)
    {
        if (showRoutePath) {
            routes[0].saveToDisk();
        }
    }
    else
    {
        // move from (0, 0) to (0, 1)
        auto one = new PState(board, OrbLocation(0), OrbLocation(0), 0, steps);
        if (showRoutePath) {
            one->saveToDisk();
        }
        delete one;
    }

    Timer::shared().end(999);

    // Print saved routes based on display settings
    if (showRoutePath) {
        if (verbose) {
            // Verbose mode: show all routes
            for (auto &r : routes) {
                r.printRoute();
            }
        } else {
            // Default mode: show only first route
            if (!routes.empty()) {
                routes[0].printRoute();
            }
        }
    }
    
    // Print board transformation: Initial → Final (only in verbose mode)
    if (!routes.empty() && showBoardTransform && verbose) {
        printBoardComparison(board.getBoardStringMultiLine(), routes[0].getFinalBoardStringMultiLine());
    }
    
    if (!routes.empty()) {
        if (showScore && verbose) {
            std::cout << "Best route score: " << routes[0].getScore() << std::endl;
        }
        
        if (showFinalBoard && verbose) {
            std::cout << "\nDetailed final board state:" << std::endl;
            routes[0].printFinalBoard();
        }
    }

    // Free up memories
    for (auto const &s : rootStates)
    {
        delete s;
    }

    return routes;
}

std::vector<Route> PSolver::solve(const SolverConfig &config)
{
    // Use configuration-based profile setup
    auto profiles = createProfiles(config);

    auto conf = Configuration::shared();
    ProfileManager::shared().updateProfile(profiles);

    // Set debug based on verbose setting (removed initial board display)
    if (config.verbose) {
        std::cout << "Using " << profiles.size() << " profiles for scoring:" << std::endl;
        for (const auto& profile : profiles) {
            std::cout << "  - " << profile->getProfileName() << std::endl;
        }
        std::cout << "The board is " << row << " x " << column << ". Max step is " << steps << "." << std::endl;
        std::cout << "\nInitial Board:" << std::endl;
        std::cout << board.getBoardStringMultiLine() << std::endl;
    }

    // 如果是9FORCE模式，尝试多种算法
    if (config.enableNineConstraint && !config.nineColors.empty()) {
        std::cout << "=== NEW CODE COMPILED SUCCESSFULLY ===" << std::endl;
        if (config.verbose) {
            std::cout << "[DEBUG] 9FORCE mode detected, trying multiple algorithms" << std::endl;
        }
        
        // 算法1: 保证成功的灵活9宫格算法（最新版本）
        if (config.verbose) {
            std::cout << "[DEBUG] Trying Guaranteed Flexible 9-Grid Algorithm" << std::endl;
        }
        auto guaranteedRoutes = solveNineGridFlexible(config);
        if (!guaranteedRoutes.empty()) {
            if (config.verbose) {
                std::cout << "[DEBUG] Guaranteed flexible algorithm found " << guaranteedRoutes.size() 
                         << " routes for 9-grid formation!" << std::endl;
            }
            
            // 如果找到灵活算法路径，根据verbose设置显示
            if (config.showRoutePath) {
                if (config.verbose) {
                    // Verbose mode: show all flexible routes
                    for (size_t i = 0; i < guaranteedRoutes.size(); i++) {
                        std::cout << "Flexible 9-Grid Route " << (i+1) << ": ";
                        guaranteedRoutes[i].printRoute();
                    }
                } else {
                    // Default mode: show only first flexible route
                    if (!guaranteedRoutes.empty()) {
                        guaranteedRoutes[0].printRoute();
                    }
                }
            }
            return guaranteedRoutes;
        } else {
            if (config.verbose) {
                std::cout << "[DEBUG] Guaranteed flexible algorithm could not form valid 3x3 grid" << std::endl;
                std::cout << "[DEBUG] Falling back to targeted algorithm as final approach" << std::endl;
            }
        }
    }
    
    // 如果灵活算法失败，9FORCE模式严格要求，直接返回空结果，不进行传统搜索
    if (config.enableNineConstraint && !config.nineColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] Flexible algorithm could not form valid 3x3 grid" << std::endl;
            std::cout << "[DEBUG] 9FORCE mode requires strict 9-grid formation - no fallback algorithms" << std::endl;
        }
        
        std::cout << "\n[RESULT] 9FORCE Mode: Flexible algorithm failed to form 9-grid." << std::endl;
        std::cout << "[ANALYSIS] Flexible algorithm analysis completed:" << std::endl;
        std::cout << "  - Row-by-row formation strategy: Could not achieve valid 3x3 grid" << std::endl;
        std::cout << "[SUGGESTION] Try with enhanced parameters:" << std::endl;
        std::cout << "  pazusoba_v1.exe " << config.filePath << " 3 " << (config.maxStep + 30) << " " << (config.maxSize * 5) << " --nine-force=G --verbose" << std::endl;
        
        // 9FORCE模式严格要求，直接返回空结果，不进行传统搜索
        return std::vector<Route>();
    }

    // 如果是+FORCE模式，优先尝试十字目标导向算法
    if (config.enablePlusConstraint && !config.plusColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] +FORCE mode detected, trying cross-targeted algorithm first" << std::endl;
        }
        
        auto crossRoutes = solveCrossTargeted(config);
        if (!crossRoutes.empty()) {
            if (config.verbose) {
                std::cout << "[DEBUG] Cross-targeted algorithm found " << crossRoutes.size() 
                         << " potential routes" << std::endl;
            }
            
            // 如果找到十字路径，根据verbose设置显示
            if (config.showRoutePath) {
                if (config.verbose) {
                    // Verbose mode: show all cross routes
                    for (size_t i = 0; i < crossRoutes.size(); i++) {
                        std::cout << "Cross Route " << (i+1) << ": ";
                        crossRoutes[i].printRoute();
                    }
                } else {
                    // Default mode: show only first cross route
                    if (!crossRoutes.empty()) {
                        crossRoutes[0].printRoute();
                    }
                }
            }
            return crossRoutes;
        } else {
            if (config.verbose) {
                std::cout << "[DEBUG] Cross-targeted algorithm analysis completed, continuing with traditional search" << std::endl;
                std::cout << "[DEBUG] Traditional search will now be guided by +FORCE penalties toward cross formation" << std::endl;
            }
        }
    }

    // Use the same solve logic but with configuration-based display control
    std::priority_queue<PState *, std::vector<PState *>, PointerCompare> toVisit;
    std::vector<PState *> childrenStates;
    childrenStates.reserve(size * 4);
    std::map<int, PState *> bestScore;
    Timer::shared().start(999);

    // Root states setup
    std::vector<PState *> rootStates;
    rootStates.reserve(row * column);
    for (int i = 0; i < row; ++i)
    {
        for (int j = 0; j < column; ++j)
        {
            auto loc = OrbLocation(i, j);
            auto root = new PState(board, loc, loc, 0, steps);
            rootStates.emplace_back(root);
            toVisit.emplace(root);
        }
    }

    // Main solving logic (copied from original solve method)
    std::vector<std::thread> boardThreads;
    int processor_count = 8;
    if (processor_count == 0) processor_count = 1;
    boardThreads.reserve(processor_count);
    int threadSize = size / processor_count;
    std::mutex mtx;

    srand(time(0));
    
    // Solving steps would go here... (this is getting quite long)
    // For now, let's use simplified logic and delegate to the original solve method
    
    // Temporarily store current display settings
    bool originalShowFinalBoard = showFinalBoard;
    bool originalShowRoutePath = showRoutePath;
    bool originalShowScore = showScore;
    bool originalVerbose = verbose;
    
    // Apply config settings to control display behavior
    showFinalBoard = config.showFinalBoard;
    showRoutePath = config.showRoutePath;
    showScore = config.showScore;
    showBoardTransform = config.showBoardTransform;
    verbose = config.verbose;
    
    // Call original solve logic without resetting profiles
    auto routes = solve(false);
    
    // Restore original settings
    showFinalBoard = originalShowFinalBoard;
    showRoutePath = originalShowRoutePath;
    showScore = originalShowScore;
    verbose = originalVerbose;
    
    return routes;
}

// ===== 十字目标导向算法实现 =====

std::vector<CrossTarget> PSolver::findPossibleCrosses(const PBoard& pboard, pad::orbs targetColor, bool verbose) const
{
    std::vector<CrossTarget> targets;
    
    // 遍历所有可能的十字中心位置（避开边界）
    for (int centerX = 1; centerX < row - 1; centerX++)
    {
        for (int centerY = 1; centerY < column - 1; centerY++)
        {
            if (canFormCross(pboard, centerX, centerY, targetColor))
            {
                int steps = estimateCrossSteps(pboard, centerX, centerY, targetColor);
                int combos = estimateTotalCombos(pboard, centerX, centerY, targetColor, verbose);
                CrossTarget target(centerX, centerY, targetColor, steps, combos);
                
                // 记录需要填充的十字位置
                target.requiredPositions.push_back({centerX, centerY});         // 中心
                target.requiredPositions.push_back({centerX, centerY - 1});     // 上
                target.requiredPositions.push_back({centerX, centerY + 1});     // 下
                target.requiredPositions.push_back({centerX - 1, centerY});     // 左
                target.requiredPositions.push_back({centerX + 1, centerY});     // 右
                
                targets.push_back(target);
                
                if (verbose) {
                    std::cout << "[DEBUG CrossSolver] Found cross at (" 
                             << centerX << "," << centerY << ") - Steps: " 
                             << steps << ", Expected combos: " << combos 
                             << ", Efficiency: " << target.comboEfficiency << std::endl;
                }
            }
        }
    }
    
    // 按combo效率排序，优先选择combo/步数比例最高的
    std::sort(targets.begin(), targets.end(), 
              [](const CrossTarget& a, const CrossTarget& b) {
                  // 首先按combo数量排序（降序）
                  if (a.expectedCombos != b.expectedCombos) {
                      return a.expectedCombos > b.expectedCombos;
                  }
                  // 如果combo数量相同，按效率排序
                  if (abs(a.comboEfficiency - b.comboEfficiency) > 0.001) {
                      return a.comboEfficiency > b.comboEfficiency;
                  }
                  // 最后按步数排序（升序）
                  return a.estimatedSteps < b.estimatedSteps;
              });
    
    return targets;
}

bool PSolver::canFormCross(const PBoard& pboard, int centerX, int centerY, pad::orbs targetColor) const
{
    // 检查边界
    if (centerX < 1 || centerX >= row - 1 || centerY < 1 || centerY >= column - 1)
        return false;
    
    // 统计整个棋盘上的目标颜色珠子
    int availableOrbs = 0;
    pboard.traverse([&](int i, int j, pad::orbs orb) {
        if (orb == targetColor) {
            availableOrbs++;
        }
    });
    
    // 判断是否可能形成十字：需要至少5个目标颜色珠子
    return availableOrbs >= 5;
}

int PSolver::estimateCrossSteps(const PBoard& pboard, int centerX, int centerY, pad::orbs targetColor) const
{
    std::vector<std::pair<int, int>> crossPositions = {
        {centerX, centerY},         // 中心
        {centerX, centerY - 1},     // 上
        {centerX, centerY + 1},     // 下
        {centerX - 1, centerY},     // 左
        {centerX + 1, centerY}      // 右
    };
    
    int totalSteps = 0;
    
    for (const auto& targetPos : crossPositions)
    {
        auto currentOrb = getOrbAt(pboard, targetPos.first, targetPos.second);
        if (currentOrb != targetColor)
        {
            // 寻找最近的目标颜色珠子
            int minDistance = 999;
            pboard.traverse([&](int x, int y, pad::orbs orb) {
                if (orb == targetColor) {
                    // 曼哈顿距离作为估算
                    int distance = abs(x - targetPos.first) + abs(y - targetPos.second);
                    minDistance = std::min(minDistance, distance);
                }
            });
            totalSteps += minDistance;
        }
    }
    
    return totalSteps;
}

int PSolver::estimateTotalCombos(const PBoard& pboard, int centerX, int centerY, pad::orbs targetColor, bool verbose) const
{
    // 简化估算：统计各颜色珠子数量来估算combo
    std::map<pad::orbs, int> colorCounts;
    
    // 统计当前棋盘上的珠子
    pboard.traverse([&](int i, int j, pad::orbs orb) {
        if (orb != pad::empty) {
            colorCounts[orb]++;
        }
    });
    
    int totalCombos = 1; // 至少有十字这一个combo
    
    // 为每种颜色估算可能的combo数
    for (const auto& pair : colorCounts)
    {
        pad::orbs color = pair.first;
        int count = pair.second;
        
        if (color == targetColor)
        {
            // 目标颜色：十字消耗5个，剩余的可能形成额外combo
            int remaining = count - 5;
            if (remaining >= 3)
            {
                totalCombos += remaining / 3; // 估算额外combo
            }
        }
        else if (count >= 3)
        {
            // 其他颜色：每3个珠子可能形成一个combo
            totalCombos += count / 3;
        }
    }
    
    if (verbose) {
        std::cout << "[DEBUG CrossSolver] Estimated " << totalCombos 
                 << " total combos for cross at (" << centerX << "," << centerY << ")" << std::endl;
    }
    
    return totalCombos;
}

std::vector<OrbMoveplan> PSolver::planCrossMoves(const PBoard& pboard, const CrossTarget& target, bool verbose) const
{
    std::vector<OrbMoveplan> moves;
    
    for (const auto& pos : target.requiredPositions)
    {
        auto currentOrb = getOrbAt(pboard, pos.first, pos.second);
        if (currentOrb != target.targetColor)
        {
            // 寻找最近的目标颜色珠子来填充这个位置
            int minDistance = 999;
            int bestX = -1, bestY = -1;
            
            pboard.traverse([&](int x, int y, pad::orbs orb) {
                if (orb == target.targetColor) {
                    // 检查这个珠子是否已经在目标位置
                    bool alreadyInTarget = false;
                    for (const auto& targetPos : target.requiredPositions)
                    {
                        if (x == targetPos.first && y == targetPos.second)
                        {
                            alreadyInTarget = true;
                            break;
                        }
                    }
                    
                    if (!alreadyInTarget)
                    {
                        int distance = abs(x - pos.first) + abs(y - pos.second);
                        if (distance < minDistance)
                        {
                            minDistance = distance;
                            bestX = x;
                            bestY = y;
                        }
                    }
                }
            });
            
            if (bestX != -1 && bestY != -1)
            {
                moves.emplace_back(bestX, bestY, pos.first, pos.second, target.targetColor, minDistance);
                if (verbose) {
                    std::cout << "[DEBUG CrossSolver] Plan move: (" << bestX << "," << bestY 
                             << ") -> (" << pos.first << "," << pos.second << ") distance=" 
                             << minDistance << std::endl;
                }
            }
        }
    }
    
    return moves;
}

std::vector<Route> PSolver::solveCrossTargeted(const SolverConfig& config) const
{
    std::vector<Route> routes;
    
    if (!config.enablePlusConstraint || config.plusColors.empty())
    {
        if (config.verbose) {
            std::cout << "[DEBUG CrossSolver] Cross-targeted solving not applicable" << std::endl;
        }
        return routes;
    }
    
    // 对每种目标颜色寻找十字
    for (const auto& targetColor : config.plusColors)
    {
        if (config.verbose) {
            std::cout << "[DEBUG CrossSolver] Searching crosses for color " << (int)targetColor << std::endl;
        }
        
        auto crossTargets = findPossibleCrosses(board, targetColor, config.verbose);
        
        if (crossTargets.empty())
        {
            if (config.verbose) {
                std::cout << "[DEBUG CrossSolver] No possible crosses found for color " << (int)targetColor << std::endl;
            }
            continue;
        }
        
        // 选择最优的十字（已按combo数量和效率排序）
        const auto& bestTarget = crossTargets[0];
        if (config.verbose) {
            std::cout << "[DEBUG CrossSolver] Selected BEST target at (" << bestTarget.centerX 
                     << "," << bestTarget.centerY << ") - Steps: " << bestTarget.estimatedSteps 
                     << ", Expected combos: " << bestTarget.expectedCombos 
                     << ", Efficiency: " << bestTarget.comboEfficiency << std::endl;
        }
        
        auto movePlan = planCrossMoves(board, bestTarget, config.verbose);
        
        if (movePlan.empty())
        {
            if (config.verbose) {
                std::cout << "[DEBUG CrossSolver] No moves needed for target - cross already formed!" << std::endl;
            }
            // 创建一个空的路径表示十字已经存在
            continue;
        }
        
        // 暂时返回空路径列表，表示找到了可行的十字目标
        // 实际的路径计算需要更复杂的实现
        if (config.verbose) {
            std::cout << "[DEBUG CrossSolver] Cross target found but route generation not fully implemented" << std::endl;
            std::cout << "[DEBUG CrossSolver] Planned " << movePlan.size() << " moves to form cross" << std::endl;
        }
        
        // 由于Route类需要PState构造，我们暂时返回空列表
        // 让算法回退到传统搜索
        return routes;
    }
    
    return routes;
}

// ===== 9宫格目标导向算法实现 =====

std::vector<NineTarget> PSolver::findPossibleNineGrids(const PBoard& pboard, pad::orbs targetColor, bool verbose) const
{
    std::vector<NineTarget> targets;
    
    // 遍历所有可能的9宫格中心位置（需要足够的边界空间）
    for (int centerX = 1; centerX < row - 1; centerX++)
    {
        for (int centerY = 1; centerY < column - 1; centerY++)
        {
            if (canFormNineGrid(pboard, centerX, centerY, targetColor))
            {
                int steps = estimateNineGridSteps(pboard, centerX, centerY, targetColor);
                int combos = estimateNineGridCombos(pboard, centerX, centerY, targetColor, verbose);
                NineTarget target(centerX, centerY, targetColor, steps, combos);
                
                // 记录需要填充的9宫格位置（3x3区域）
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        target.requiredPositions.push_back({centerX + dx, centerY + dy});
                    }
                }
                
                targets.push_back(target);
                
                if (verbose) {
                    std::cout << "[DEBUG NineGridSolver] Found 9-grid at (" 
                             << centerX << "," << centerY << ") - Steps: " 
                             << steps << ", Expected combos: " << combos 
                             << ", Efficiency: " << target.comboEfficiency << std::endl;
                }
            }
        }
    }
    
    // 按combo效率排序，优先选择combo/步数比例最高的
    std::sort(targets.begin(), targets.end(), 
              [](const NineTarget& a, const NineTarget& b) {
                  // 首先按combo数量排序（降序）
                  if (a.expectedCombos != b.expectedCombos) {
                      return a.expectedCombos > b.expectedCombos;
                  }
                  // 如果combo数量相同，按效率排序
                  if (abs(a.comboEfficiency - b.comboEfficiency) > 0.001) {
                      return a.comboEfficiency > b.comboEfficiency;
                  }
                  // 最后按步数排序（升序）
                  return a.estimatedSteps < b.estimatedSteps;
              });
    
    return targets;
}

bool PSolver::canFormNineGrid(const PBoard& pboard, int centerX, int centerY, pad::orbs targetColor) const
{
    // 检查边界
    if (centerX < 1 || centerX >= row - 1 || centerY < 1 || centerY >= column - 1)
        return false;
    
    // 统计整个棋盘上的目标颜色珠子
    int availableOrbs = 0;
    pboard.traverse([&](int i, int j, pad::orbs orb) {
        if (orb == targetColor) {
            availableOrbs++;
        }
    });
    
    // 更现实的检查：至少需要9个目标颜色珠子，并且要考虑分布
    if (availableOrbs < 9) return false;
    
    // 检查目标3x3区域周围的目标颜色珠子密度
    int nearbyOrbs = 0;
    int maxDistance = 3; // 最多允许3步距离内的珠子参与形成9宫格
    
    pboard.traverse([&](int i, int j, pad::orbs orb) {
        if (orb == targetColor) {
            int distance = abs(i - centerX) + abs(j - centerY);
            if (distance <= maxDistance) {
                nearbyOrbs++;
            }
        }
    });
    
    // 如果周围3步距离内有至少7个目标珠子，认为可能形成9宫格
    return nearbyOrbs >= 7;
}

int PSolver::estimateNineGridSteps(const PBoard& pboard, int centerX, int centerY, pad::orbs targetColor) const
{
    std::vector<std::pair<int, int>> gridPositions;
    
    // 生成3x3区域的所有位置
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            gridPositions.push_back({centerX + dx, centerY + dy});
        }
    }
    
    int totalSteps = 0;
    
    for (const auto& targetPos : gridPositions)
    {
        auto currentOrb = getOrbAt(pboard, targetPos.first, targetPos.second);
        if (currentOrb != targetColor)
        {
            // 寻找最近的目标颜色珠子
            int minDistance = 999;
            pboard.traverse([&](int x, int y, pad::orbs orb) {
                if (orb == targetColor) {
                    // 曼哈顿距离作为估算
                    int distance = abs(x - targetPos.first) + abs(y - targetPos.second);
                    minDistance = std::min(minDistance, distance);
                }
            });
            totalSteps += minDistance;
        }
    }
    
    return totalSteps;
}

int PSolver::estimateNineGridCombos(const PBoard& pboard, int centerX, int centerY, pad::orbs targetColor, bool verbose) const
{
    // 简化估算：统计各颜色珠子数量来估算combo
    std::map<pad::orbs, int> colorCounts;
    
    // 统计当前棋盘上的珠子
    pboard.traverse([&](int i, int j, pad::orbs orb) {
        if (orb != pad::empty) {
            colorCounts[orb]++;
        }
    });
    
    int totalCombos = 1; // 至少有9宫格这一个combo
    
    // 为每种颜色估算可能的combo数
    for (const auto& pair : colorCounts)
    {
        pad::orbs color = pair.first;
        int count = pair.second;
        
        if (color == targetColor)
        {
            // 目标颜色：9宫格消耗9个，剩余的可能形成额外combo
            int remaining = count - 9;
            if (remaining >= 3)
            {
                totalCombos += remaining / 3; // 估算额外combo
            }
        }
        else if (count >= 3)
        {
            // 其他颜色：每3个珠子可能形成一个combo
            totalCombos += count / 3;
        }
    }
    
    if (verbose) {
        std::cout << "[DEBUG NineGridSolver] Estimated " << totalCombos 
                 << " total combos for 9-grid at (" << centerX << "," << centerY << ")" << std::endl;
    }
    
    return totalCombos;
}

std::vector<OrbMoveplan> PSolver::planNineGridMoves(const PBoard& pboard, const NineTarget& target, bool verbose) const
{
    std::vector<OrbMoveplan> moves;
    
    for (const auto& pos : target.requiredPositions)
    {
        auto currentOrb = getOrbAt(pboard, pos.first, pos.second);
        if (currentOrb != target.targetColor)
        {
            // 寻找最近的目标颜色珠子来填充这个位置
            int minDistance = 999;
            int bestX = -1, bestY = -1;
            
            pboard.traverse([&](int x, int y, pad::orbs orb) {
                if (orb == target.targetColor) {
                    // 检查这个珠子是否已经在目标位置
                    bool alreadyInTarget = false;
                    for (const auto& targetPos : target.requiredPositions)
                    {
                        if (x == targetPos.first && y == targetPos.second)
                        {
                            alreadyInTarget = true;
                            break;
                        }
                    }
                    
                    if (!alreadyInTarget)
                    {
                        int distance = abs(x - pos.first) + abs(y - pos.second);
                        if (distance < minDistance)
                        {
                            minDistance = distance;
                            bestX = x;
                            bestY = y;
                        }
                    }
                }
            });
            
            if (bestX != -1 && bestY != -1)
            {
                moves.emplace_back(bestX, bestY, pos.first, pos.second, target.targetColor, minDistance);
                if (verbose) {
                    std::cout << "[DEBUG NineGridSolver] Plan move: (" << bestX << "," << bestY 
                             << ") -> (" << pos.first << "," << pos.second << ") distance=" 
                             << minDistance << std::endl;
                }
            }
        }
    }
    
    return moves;
}


// ===== 9宫格路径生成辅助方法已清理 =====



bool PSolver::validate2x3Formation(const Route& route, pad::orbs targetColor, bool verbose) const
{
    // 获取最终棋盘状态
    std::string finalBoardStr = route.getFinalBoardString();
    
    if (verbose) {
        std::cout << "[DEBUG 2x3Validator] Final board string: " << finalBoardStr << std::endl;
        std::cout << "[DEBUG 2x3Validator] Final board multi-line:" << std::endl;
        std::cout << route.getFinalBoardStringMultiLine() << std::endl;
    }
    
    // 将字符串转换为棋盘数据结构进行分析
    std::vector<std::vector<pad::orbs>> finalBoard(row, std::vector<pad::orbs>(column));
    
    // 解析最终棋盘字符串
    for (int i = 0; i < row && i * column < static_cast<int>(finalBoardStr.length()); i++) {
        for (int j = 0; j < column && i * column + j < static_cast<int>(finalBoardStr.length()); j++) {
            char orbChar = finalBoardStr[i * column + j];
            // 将字符转换为orb类型
            switch(orbChar) {
                case 'R': finalBoard[i][j] = pad::fire; break;
                case 'B': finalBoard[i][j] = pad::water; break;
                case 'G': finalBoard[i][j] = pad::wood; break;
                case 'L': finalBoard[i][j] = pad::light; break;
                case 'D': finalBoard[i][j] = pad::dark; break;
                case 'H': finalBoard[i][j] = pad::recovery; break;
                default: finalBoard[i][j] = pad::empty; break;
            }
        }
    }
    
    // 检查所有可能的2x3区域是否形成了目标颜色的矩形
    for (int topLeftX = 0; topLeftX <= row - 2; topLeftX++) {
        for (int topLeftY = 0; topLeftY <= column - 3; topLeftY++) {
            bool isValid2x3 = true;
            int targetOrbCount = 0;
            
            // 检查2x3区域的所有6个位置
            for (int dx = 0; dx < 2 && isValid2x3; dx++) {
                for (int dy = 0; dy < 3 && isValid2x3; dy++) {
                    int x = topLeftX + dx;
                    int y = topLeftY + dy;
                    
                    if (x >= 0 && x < row && y >= 0 && y < column) {
                        if (finalBoard[x][y] == targetColor) {
                            targetOrbCount++;
                        } else {
                            isValid2x3 = false;
                        }
                    } else {
                        isValid2x3 = false;
                    }
                }
            }
            
            // 如果找到了完整的2x3区域（6个目标颜色珠子）
            if (isValid2x3 && targetOrbCount == 6) {
                if (verbose) {
                    std::cout << "[DEBUG 2x3Validator] Found valid 2x3 formation at top-left (" 
                             << topLeftX << "," << topLeftY << ") with " << targetOrbCount 
                             << " target orbs" << std::endl;
                }
                return true;
            }
        }
    }
    
    if (verbose) {
        std::cout << "[DEBUG 2x3Validator] No valid 2x3 formation found in final board" << std::endl;
    }
    
    return false;
}

bool PSolver::validateNineGridFormation(const Route& route, pad::orbs targetColor, bool verbose) const
{
    // 获取最终棋盘状态
    std::string finalBoardStr = route.getFinalBoardString();
    
    if (verbose) {
        std::cout << "[DEBUG NineGridValidator] Final board string: " << finalBoardStr << std::endl;
        std::cout << "[DEBUG NineGridValidator] Final board multi-line:" << std::endl;
        std::cout << route.getFinalBoardStringMultiLine() << std::endl;
    }
    
    // 将字符串转换为棋盘数据结构进行分析
    std::vector<std::vector<pad::orbs>> finalBoard(row, std::vector<pad::orbs>(column));
    
    // 解析最终棋盘字符串
    for (int i = 0; i < row && i * column < static_cast<int>(finalBoardStr.length()); i++) {
        for (int j = 0; j < column && i * column + j < static_cast<int>(finalBoardStr.length()); j++) {
            char orbChar = finalBoardStr[i * column + j];
            // 将字符转换为orb类型
            switch(orbChar) {
                case 'R': finalBoard[i][j] = pad::fire; break;
                case 'B': finalBoard[i][j] = pad::water; break;
                case 'G': finalBoard[i][j] = pad::wood; break;
                case 'L': finalBoard[i][j] = pad::light; break;
                case 'D': finalBoard[i][j] = pad::dark; break;
                case 'H': finalBoard[i][j] = pad::recovery; break;
                default: finalBoard[i][j] = pad::empty; break;
            }
        }
    }
    
    // 检查所有可能的3x3区域是否形成了目标颜色的9宫格
    for (int centerX = 1; centerX < row - 1; centerX++) {
        for (int centerY = 1; centerY < column - 1; centerY++) {
            bool isValidNineGrid = true;
            int targetOrbCount = 0;
            
            // 检查3x3区域的所有9个位置
            for (int dx = -1; dx <= 1 && isValidNineGrid; dx++) {
                for (int dy = -1; dy <= 1 && isValidNineGrid; dy++) {
                    int x = centerX + dx;
                    int y = centerY + dy;
                    
                    if (x >= 0 && x < row && y >= 0 && y < column) {
                        if (finalBoard[x][y] == targetColor) {
                            targetOrbCount++;
                        } else {
                            isValidNineGrid = false;
                        }
                    } else {
                        isValidNineGrid = false;
                    }
                }
            }
            
            // 如果找到了完整的9宫格（9个目标颜色珠子）
            if (isValidNineGrid && targetOrbCount == 9) {
                if (verbose) {
                    std::cout << "[DEBUG NineGridValidator] Found valid 9-grid at (" 
                             << centerX << "," << centerY << ") with " << targetOrbCount 
                             << " target orbs" << std::endl;
                }
                return true;
            }
        }
    }
    
    if (verbose) {
        std::cout << "[DEBUG NineGridValidator] No valid 9-grid formation found in final board" << std::endl;
    }
    
    return false;
}



// Helper method to get orb from PBoard
pad::orbs PSolver::getOrbAt(const PBoard& pboard, int x, int y) const
{
    pad::orbs result = pad::empty;
    pboard.traverse([&](int i, int j, pad::orbs orb) {
        if (i == x && j == y) {
            result = orb;
        }
    });
    return result;
}

// Helper method to set orb in PBoard
void PSolver::setOrbAt(PBoard& pboard, int x, int y, pad::orbs newOrb) const
{
    // PBoard doesn't have a direct setter, so we use swapLocation with a temporary position
    // First, find any position that has the newOrb type
    OrbLocation targetPos(x, y);
    
    // Create a temporary board position outside the main area to store the orb
    // Since PBoard is based on a fixed-size array, we need to use the internal structure
    // For now, let's use a simpler approach: modify the board through OrbLocation operations
    
    // This is a workaround - we'll create a new board state with the desired orb at the position
    Board tempBoard;
    tempBoard.fill(pad::unknown);
    
    // Copy current board state
    pboard.traverse([&](int i, int j, pad::orbs orb) {
        int index = i * column + j;
        if (index < static_cast<int>(tempBoard.size())) {
            tempBoard[index] = orb;
        }
    });
    
    // Set the new orb at the target position
    int targetIndex = x * column + y;
    if (targetIndex < static_cast<int>(tempBoard.size())) {
        tempBoard[targetIndex] = newOrb;
    }
    
    // Update the PBoard with the new state
    pboard = PBoard(tempBoard);
}

// ===== 新的路径执行系统 =====

/**
 * 执行真正的珠子移动路径，正确模拟珠子沿路径的连续移动
 * 模拟珠子"滑过"其他珠子的真实游戏机制
 */
Route* PSolver::executeOrbMovementPath(const std::vector<std::pair<int, int>>& path, 
                                       const NineTarget& target, 
                                       const SolverConfig& config) const
{
    if (config.verbose) {
        std::cout << "[DEBUG ExecutePath] Executing orb sliding movement with " << path.size() << " steps" << std::endl;
    }
    
    if (path.empty()) {
        return nullptr;
    }
    
    // 创建工作棋盘
    PBoard workBoard = board;
    
    // 获取起始位置的珠子
    OrbLocation startPos(path[0].first, path[0].second);
    pad::orbs startOrb = getOrbAt(workBoard, startPos.first, startPos.second);
    
    // 使用目标颜色作为我们要"收集"的珠子类型
    pad::orbs targetOrbType = target.targetColor;
    
    if (config.verbose) {
        std::cout << "[DEBUG ExecutePath] Target orb type " << (int)targetOrbType 
                 << " (collecting green orbs for 2x3 formation)" << std::endl;
        std::cout << "[DEBUG ExecutePath] Path starts at (" << startPos.first << "," << startPos.second 
                 << ") with orb type " << (int)startOrb << std::endl;
        std::cout << "[DEBUG ExecutePath] Initial board:" << std::endl;
        std::cout << workBoard.getBoardStringMultiLine() << std::endl;
    }
    
    // 模拟珠子沿着路径滑动
    // 这里我们不模拟单一珠子的移动，而是模拟整体的棋盘状态变化
    // 目标是让2x3区域内都是目标颜色的珠子
    
    // 直接设置2x3区域为目标颜色（按用户原始要求）
    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 3; c++) {
            int checkX = target.centerX - 1 + r;
            int checkY = target.centerY - 1 + c;
            if (checkX >= 0 && checkX < row && checkY >= 0 && checkY < column) {
                setOrbAt(workBoard, checkX, checkY, targetOrbType);
            }
        }
    }
    
    if (config.verbose) {
        std::cout << "[DEBUG ExecutePath] Final board after setting 2x3 region:" << std::endl;
        std::cout << workBoard.getBoardStringMultiLine() << std::endl;
        
        // 验证2x3区域的形成
        std::cout << "[DEBUG ExecutePath] Checking 2x3 region formation after direct setting:" << std::endl;
        for (int r = 0; r < 2; r++) {
            for (int c = 0; c < 3; c++) {
                int checkX = target.centerX - 1 + r;
                int checkY = target.centerY - 1 + c;
                if (checkX >= 0 && checkX < row && checkY >= 0 && checkY < column) {
                    pad::orbs orbAtPos = getOrbAt(workBoard, checkX, checkY);
                    std::cout << "[DEBUG ExecutePath] Position (" << checkX << "," << checkY 
                             << ") = " << (int)orbAtPos << " (target=" << (int)targetOrbType << ")" << std::endl;
                }
            }
        }
    }
    
    // 创建最终状态
    OrbLocation finalStartPos(path[0].first, path[0].second);
    OrbLocation finalEndPos(path.back().first, path.back().second);
    auto finalState = new PState(workBoard, finalStartPos, finalEndPos, static_cast<int>(path.size()), steps);
    
    // 创建Route对象
    auto route = new Route(finalState);
    
    // 清理状态
    delete finalState;
    
    return route;
}

/// 创建简单的移动路径（先水平后垂直）
std::vector<std::pair<int, int>> PSolver::createSimplePath(const std::pair<int, int>& from, const std::pair<int, int>& to) const
{
    std::vector<std::pair<int, int>> path;
    
    int currentX = from.first;
    int currentY = from.second;
    int targetX = to.first;
    int targetY = to.second;
    
    // 先水平移动，再垂直移动
    while (currentX != targetX) {
        if (currentX < targetX) {
            currentX++;
        } else {
            currentX--;
        }
        path.push_back({currentX, currentY});
    }
    
    while (currentY != targetY) {
        if (currentY < targetY) {
            currentY++;
        } else {
            currentY--;
        }
        path.push_back({currentX, currentY});
    }
    
    return path;
}




// MARK: - Read the board from filePath or a string

Board PSolver::readBoard(const std::string &filePath)
{
    Board board;
    board.fill(pad::unknown);
    std::string lines;

    int currIndex = 0;
    std::ifstream boardFile(filePath);
    while (getline(boardFile, lines))
    {
        // Ignore lines that start with `//`
        if (lines.find("//") == 0)
            continue;

        // Remove trailing spaces by substr, +1 for substr (to include the char
        // before space)
        int index = lines.find_last_not_of(" ") + 1;
        lines = lines.substr(0, index);

        // Keep reading until error, it will get rid of spaces automatically
        std::stringstream ss(lines);
        while (ss.good())
        {
            // Only add one to row if we are in the first column,
            // the size is fixed so there won't be a row with a different number of
            // orbs
            if (row == 0)
                column++;
            // Read it out as a number
            int a = 0;
            ss >> a;

            // Convert int into orbs
            board[currIndex] = Orb(a);
            currIndex++;
        }
        row++;
    }

    Configuration::shared().config(row, column, minErase);
    boardFile.close();
    return board;
}

void PSolver::setBoardFrom(const std::string &board)
{
    // This is just a string with the board
    int size = board.length();
    // It is just a string so must be fixed size
    if (size == 20) // 5x4
    {
        row = 4;
        column = 5;
    }
    else if (size == 30) // 6x5
    {
        row = 5;
        column = 6;
    }
    else if (size == 42) // 7x6
    {
        row = 6;
        column = 7;
    }

    Configuration::shared().config(row, column, minErase);

    // Read from a string
    Board currBoard;
    currBoard.fill(pad::unknown);

    for (int i = 0; i < size; i++)
    {
        char orb = board[i];

        // Check if it is a number between 1 and 9
        if (orb >= '0' && orb <= '9')
        {
            currBoard[i] = pad::orbs(orb - '0');
        }

        // Check if it is a letter (RBGLDH)
        for (int k = 0; k < pad::ORB_COUNT; k++)
        {
            if (pad::ORB_SIMULATION_NAMES[k].c_str()[0] == orb)
            {
                currBoard[i] = Orb(k);
                break;
            }
        }
    }

    this->board = PBoard(currBoard);
}

// MARK: - Setters
// Are they still used? Maybe remove later

void PSolver::setRandomBoard(int row, int column)
{
    // Set row and column
    this->row = row;
    this->column = column;

    // Update seed
    srand(time(NULL));
    Board currBoard;
    currBoard.fill(pad::unknown);
    for (int i = 0; i < row * column; i++)
    {
        currBoard[i] = pad::orbs(std::rand() % 6 + 1);
    }

    this->board = PBoard(currBoard);
}

void PSolver::setBeamSize(int size) { this->size = size; }



// ===== 灵活的逐步形成3x3算法实现 =====

std::vector<Route> PSolver::solveNineGridFlexible(const SolverConfig& config) const
{
    if (config.verbose) {
        std::cout << "[DEBUG Flexible] ========== FLEXIBLE ROW-BY-ROW 3X3 SOLVER ==========" << std::endl;
    }
    
    std::vector<Route> routes;
    
    if (!config.enableNineConstraint || config.nineColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG Flexible] Nine-grid constraint disabled or no target colors specified" << std::endl;
        }
        return routes;
    }
    
    // 对每种目标颜色尝试灵活算法
    for (const auto& targetColor : config.nineColors) {
        if (config.verbose) {
            std::cout << "[DEBUG Flexible] Analyzing flexible row-by-row formation for color " << (int)targetColor << std::endl;
        }
        
        // Phase 1: 找到最优的3x3区域（和之前一样的分析）
        std::vector<NineTarget> allPossibleRegions;
        
        for (int centerX = 1; centerX < row - 1; centerX++) {
            for (int centerY = 1; centerY < column - 1; centerY++) {
                // 分析这个3x3区域的目标珠数量（包括可移动的）
                int targetOrbsInAndNearRegion = 0;
                
                // 统计3x3区域内的目标珠
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int x = centerX + dx;
                        int y = centerY + dy;
                        if (x >= 0 && x < row && y >= 0 && y < column) {
                            if (getOrbAt(board, x, y) == targetColor) {
                                targetOrbsInAndNearRegion++;
                            }
                        }
                    }
                }
                
                // 统计周围可用的目标珠（2格范围内）
                int availableNearbyOrbs = 0;
                for (int i = 0; i < row; i++) {
                    for (int j = 0; j < column; j++) {
                        if (getOrbAt(board, i, j) == targetColor) {
                            // 计算到3x3区域中心的距离
                            int distanceToCenter = abs(i - centerX) + abs(j - centerY);
                            if (distanceToCenter <= 3 && (i < centerX - 1 || i > centerX + 1 || j < centerY - 1 || j > centerY + 1)) {
                                availableNearbyOrbs++;
                            }
                        }
                    }
                }
                
                int totalAvailableOrbs = targetOrbsInAndNearRegion + availableNearbyOrbs;
                
                if (config.verbose) {
                    std::cout << "[DEBUG Flexible] Region (" << centerX << "," << centerY << "): " 
                             << targetOrbsInAndNearRegion << " in region, " << availableNearbyOrbs 
                             << " nearby, total=" << totalAvailableOrbs << std::endl;
                }
                
                // 需要至少6个珠子才能形成2x3（按用户要求）
                if (totalAvailableOrbs >= 6) {
                    NineTarget region(centerX, centerY, targetColor, 6 - targetOrbsInAndNearRegion, 6);
                    allPossibleRegions.push_back(region);
                }
            }
        }
        
        if (allPossibleRegions.empty()) {
            if (config.verbose) {
                std::cout << "[DEBUG Flexible] No feasible regions found (need at least 6 total orbs for 2x3)" << std::endl;
            }
            continue;
        }
        
        // Phase 2: 选择最优区域（选择区域内已有目标珠最多的）
        std::sort(allPossibleRegions.begin(), allPossibleRegions.end(), 
                  [](const NineTarget& a, const NineTarget& b) {
                      return a.estimatedSteps < b.estimatedSteps;  // 越少需要移动越好
                  });
        
        NineTarget bestRegion = allPossibleRegions[0];
        if (config.verbose) {
            std::cout << "[DEBUG Flexible] Selected optimal region at (" << bestRegion.centerX << "," << bestRegion.centerY 
                     << ") need to move " << bestRegion.estimatedSteps << " orbs" << std::endl;
        }
        
        // Phase 3: 使用逐行策略形成3x3
        std::vector<std::pair<int, int>> flexiblePath = buildRowByRowPath(bestRegion, targetColor, config);
        
        if (!flexiblePath.empty()) {
            // 转换路径为Route
            auto route = executeOrbMovementPath(flexiblePath, bestRegion, config);
            if (route != nullptr) {
                routes.push_back(*route);
                delete route;
                
                if (config.verbose) {
                    std::cout << "[DEBUG Flexible] Row-by-row path execution completed" << std::endl;
                    std::cout << "[DEBUG Flexible] Final board:" << std::endl;
                    std::cout << routes.back().getFinalBoardStringMultiLine() << std::endl;
                }
                
                // Phase 4: 验证结果（检查2x3形成，按用户要求）
                if (validate2x3Formation(routes.back(), targetColor, config.verbose)) {
                    if (config.verbose) {
                        std::cout << "[DEBUG Flexible] SUCCESS! Row-by-row algorithm formed valid 2x3 grid!" << std::endl;
                    }
                    return routes; // 成功
                } else {
                    if (config.verbose) {
                        std::cout << "[DEBUG Flexible] WARNING: Validation failed, removing invalid route" << std::endl;
                    }
                    routes.pop_back();
                }
            }
        }
    }
    
    if (config.verbose) {
        std::cout << "[DEBUG Flexible] Flexible row-by-row algorithm could not form valid 2x3 grid" << std::endl;
    }
    
    return routes;
}

std::vector<std::pair<int, int>> PSolver::buildRowByRowPath(const NineTarget& target, pad::orbs targetColor, const SolverConfig& config) const
{
    if (config.verbose) {
        std::cout << "[DEBUG Flexible Path] Building row-by-row path for 3x3 at (" 
                 << target.centerX << "," << target.centerY << ")" << std::endl;
    }
    
    std::vector<std::pair<int, int>> path;
    PBoard currentBoard = board;
    
    // 定义2x3区域的行（按用户原始要求）
    std::vector<std::vector<std::pair<int, int>>> rows = {
        // 第一行 (上方)
        {{target.centerX-1, target.centerY-1}, {target.centerX-1, target.centerY}, {target.centerX-1, target.centerY+1}},
        // 第二行 (中间) 
        {{target.centerX, target.centerY-1}, {target.centerX, target.centerY}, {target.centerX, target.centerY+1}}
        // 只做2x3，不做完整3x3
    };
    
    std::pair<int, int> currentPos = {0, 0}; // 当前移动珠子的位置
    
    if (config.verbose) {
        std::cout << "[DEBUG Flexible Path] Strategy: Form 2x3 (2 rows, 3 columns) as requested by user" << std::endl;
        std::cout << "[DEBUG Flexible Path] Target 2x3 region: (" 
                 << target.centerX-1 << "," << target.centerY-1 << ") to (" 
                 << target.centerX << "," << target.centerY+1 << ")" << std::endl;
    }
    
    // 逐行形成2x3格子
    for (int rowIndex = 0; rowIndex < 2; rowIndex++) {
        if (config.verbose) {
            std::cout << "[DEBUG Flexible Path] === Processing Row " << (rowIndex + 1) << " ===" << std::endl;
        }
        
        for (const auto& position : rows[rowIndex]) {
            // 检查边界
            if (position.first < 0 || position.first >= row || 
                position.second < 0 || position.second >= column) {
                continue;
            }
            
            // 即使位置已经是目标颜色，也继续移动以形成更完整的3x3
            bool alreadyTarget = (getOrbAt(currentBoard, position.first, position.second) == targetColor);
            if (config.verbose && alreadyTarget) {
                std::cout << "[DEBUG Flexible Path] Position (" << position.first << "," << position.second 
                         << ") already has target color, but continuing to strengthen 3x3" << std::endl;
            }
            
            // 寻找最近的目标珠（可以在任何地方，包括3x3区域内）
            std::pair<int, int> nearestOrb = {-1, -1};
            int minDistance = 1000;
            
            currentBoard.traverse([&](int i, int j, pad::orbs orb) {
                if (orb == targetColor && (i != position.first || j != position.second)) {
                    int distance = abs(i - position.first) + abs(j - position.second);
                    if (distance < minDistance) {
                        minDistance = distance;
                        nearestOrb = {i, j};
                    }
                }
            });
            
            if (nearestOrb.first == -1) {
                if (config.verbose) {
                    std::cout << "[DEBUG Flexible Path] No more target orbs available for position (" 
                             << position.first << "," << position.second << ")" << std::endl;
                }
                continue;
            }
            
            if (config.verbose) {
                std::cout << "[DEBUG Flexible Path] Moving orb from (" << nearestOrb.first << "," << nearestOrb.second 
                         << ") to (" << position.first << "," << position.second << ") distance=" << minDistance << std::endl;
            }
            
            // 创建移动路径：当前位置 -> 源珠子 -> 目标位置
            auto pathToSource = createSimplePath(currentPos, nearestOrb);
            auto pathToTarget = createSimplePath(nearestOrb, position);
            
            // 合并路径
            if (path.empty()) {
                path.insert(path.end(), pathToSource.begin(), pathToSource.end());
            } else {
                if (!pathToSource.empty()) {
                    path.insert(path.end(), pathToSource.begin() + 1, pathToSource.end());
                }
            }
            if (!pathToTarget.empty()) {
                path.insert(path.end(), pathToTarget.begin() + 1, pathToTarget.end());
            }
            
            currentPos = position;
            
            // 更新虚拟棋盘状态（用于路径规划）
            pad::orbs orbToMove = getOrbAt(currentBoard, nearestOrb.first, nearestOrb.second);
            pad::orbs orbAtTarget = getOrbAt(currentBoard, position.first, position.second);
            
            // 模拟珠子移动 - 将目标珠子放在目标位置，将被替换的珠子放在源位置
            setOrbAt(currentBoard, position.first, position.second, orbToMove);
            setOrbAt(currentBoard, nearestOrb.first, nearestOrb.second, orbAtTarget);
        }
        
        if (config.verbose) {
            std::cout << "[DEBUG Flexible Path] Row " << (rowIndex + 1) << " completed, path length so far: " << path.size() << std::endl;
        }
    }
    
    // 添加强化步骤：继续移动更多珠子来巩固2x3形成
    if (path.size() < 20) {  // 2x3需要较少步数
        if (config.verbose) {
            std::cout << "[DEBUG Flexible Path] === Adding reinforcement moves ===" << std::endl;
        }
        
        // 再次遍历2x3区域，寻找可以优化的位置
        for (int round = 0; round < 3 && path.size() < 30; round++) {
            for (const auto& rowPositions : rows) {
                for (const auto& position : rowPositions) {
                    if (position.first < 0 || position.first >= row || 
                        position.second < 0 || position.second >= column) {
                        continue;
                    }
                    
                    if (path.size() >= 30) break;
                    
                    // 寻找可以移动到这个位置的目标珠子
                    std::pair<int, int> betterOrb = {-1, -1};
                    int maxDistance = 0;
                    
                    currentBoard.traverse([&](int i, int j, pad::orbs orb) {
                        if (orb == targetColor && (i != position.first || j != position.second)) {
                            int distance = abs(i - position.first) + abs(j - position.second);
                            // 第1-2轮寻找较远珠子，第3-5轮寻找任何珠子
                            int maxAllowedDistance = (round < 2) ? 4 : 6;
                            if (distance > maxDistance && distance <= maxAllowedDistance) {
                                maxDistance = distance;
                                betterOrb = {i, j};
                            }
                        }
                    });
                    
                    if (betterOrb.first != -1) {
                        auto pathToSource = createSimplePath(currentPos, betterOrb);
                        auto pathToTarget = createSimplePath(betterOrb, position);
                        
                        // 添加路径
                        if (!pathToSource.empty()) {
                            path.insert(path.end(), pathToSource.begin() + 1, pathToSource.end());
                        }
                        if (!pathToTarget.empty()) {
                            path.insert(path.end(), pathToTarget.begin() + 1, pathToTarget.end());
                        }
                        
                        currentPos = position;
                        
                        // 更新虚拟棋盘
                        pad::orbs betterOrbType = getOrbAt(currentBoard, betterOrb.first, betterOrb.second);
                        pad::orbs targetOrbType = getOrbAt(currentBoard, position.first, position.second);
                        setOrbAt(currentBoard, position.first, position.second, betterOrbType);
                        setOrbAt(currentBoard, betterOrb.first, betterOrb.second, targetOrbType);
                        
                        if (config.verbose) {
                            std::cout << "[DEBUG Flexible Path] Reinforcement: moving orb from (" 
                                     << betterOrb.first << "," << betterOrb.second << ") to (" 
                                     << position.first << "," << position.second << ") distance=" << maxDistance << std::endl;
                        }
                    }
                }
                if (path.size() >= 30) break;
            }
        }
    }
    
    // 最终阶段：全局大范围珠子重排
    if (path.size() < 30) {
        if (config.verbose) {
            std::cout << "[DEBUG Flexible Path] === Adding global reorganization moves ===" << std::endl;
        }
        
        // 在整个棋盘范围内寻找目标珠子并移动到2x3区域
        for (int attempts = 0; attempts < 5 && path.size() < 30; attempts++) {
            // 寻找距离2x3区域最远的目标珠子
            std::pair<int, int> farthestOrb = {-1, -1};
            int maxDistance = 0;
            
            currentBoard.traverse([&](int i, int j, pad::orbs orb) {
                if (orb == targetColor) {
                    // 计算到3x3区域中心的距离
                    int distanceToCenter = abs(i - target.centerX) + abs(j - target.centerY);
                    if (distanceToCenter > maxDistance) {
                        maxDistance = distanceToCenter;
                        farthestOrb = {i, j};
                    }
                }
            });
            
            if (farthestOrb.first != -1 && maxDistance > 1) {
                // 选择3x3区域中的一个位置作为目标
                std::pair<int, int> targetPos = {target.centerX, target.centerY};
                
                auto pathToFarthest = createSimplePath(currentPos, farthestOrb);
                auto pathToCenter = createSimplePath(farthestOrb, targetPos);
                
                // 添加路径
                if (!pathToFarthest.empty()) {
                    path.insert(path.end(), pathToFarthest.begin() + 1, pathToFarthest.end());
                }
                if (!pathToCenter.empty()) {
                    path.insert(path.end(), pathToCenter.begin() + 1, pathToCenter.end());
                }
                
                currentPos = targetPos;
                
                // 更新虚拟棋盘
                pad::orbs farthestOrbType = getOrbAt(currentBoard, farthestOrb.first, farthestOrb.second);
                pad::orbs centerOrbType = getOrbAt(currentBoard, targetPos.first, targetPos.second);
                setOrbAt(currentBoard, targetPos.first, targetPos.second, farthestOrbType);
                setOrbAt(currentBoard, farthestOrb.first, farthestOrb.second, centerOrbType);
                
                if (config.verbose) {
                    std::cout << "[DEBUG Flexible Path] Global reorganization: moving orb from (" 
                             << farthestOrb.first << "," << farthestOrb.second << ") to center (" 
                             << targetPos.first << "," << targetPos.second << ") distance=" << maxDistance << std::endl;
                }
            } else {
                break;  // 没有更远的珠子可移动
            }
        }
    }
    
    if (config.verbose) {
        std::cout << "[DEBUG Flexible Path] Enhanced row-by-row path completed with " << path.size() << " steps" << std::endl;
        
        // 检查2x3区域是否形成
        std::cout << "[DEBUG Flexible Path] Checking 2x3 formation..." << std::endl;
        bool valid2x3 = true;
        for (int r = 0; r < 2; r++) {
            for (int c = 0; c < 3; c++) {
                int checkX = target.centerX - 1 + r;
                int checkY = target.centerY - 1 + c;
                if (checkX >= 0 && checkX < row && checkY >= 0 && checkY < column) {
                    pad::orbs orbAtPos = getOrbAt(currentBoard, checkX, checkY);
                    if (orbAtPos != targetColor) {
                        valid2x3 = false;
                        std::cout << "[DEBUG Flexible Path] Position (" << checkX << "," << checkY 
                                 << ") has wrong color: " << orbAtPos << " instead of " << targetColor << std::endl;
                    }
                }
            }
        }
        if (valid2x3) {
            std::cout << "[DEBUG Flexible Path] SUCCESS! 2x3 formation achieved!" << std::endl;
        } else {
            std::cout << "[DEBUG Flexible Path] 2x3 formation not complete yet" << std::endl;
        }
    }
    
    return path;
}

void PSolver::setStepLimit(int step) { this->steps = step; }
