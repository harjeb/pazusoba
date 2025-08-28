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

    // 如果是9FORCE模式，直接使用分布式聚集算法
    if (config.enableNineConstraint && !config.nineColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] 9FORCE mode detected, using distributed clustering algorithm" << std::endl;
        }
        
        auto distributedRoutes = solveNineGridDistributed(config);
        if (!distributedRoutes.empty()) {
            if (config.verbose) {
                std::cout << "[DEBUG] Distributed clustering found " << distributedRoutes.size() 
                         << " routes for 9-grid formation!" << std::endl;
            }
            
            // 如果找到分布式路径，根据verbose设置显示
            if (config.showRoutePath) {
                if (config.verbose) {
                    // Verbose mode: show all distributed routes
                    for (size_t i = 0; i < distributedRoutes.size(); i++) {
                        std::cout << "Distributed 9-Grid Route " << (i+1) << ": ";
                        distributedRoutes[i].printRoute();
                    }
                } else {
                    // Default mode: show only first distributed route
                    if (!distributedRoutes.empty()) {
                        distributedRoutes[0].printRoute();
                    }
                }
            }
            return distributedRoutes;
        } else {
            if (config.verbose) {
                std::cout << "[DEBUG] Distributed clustering completed but no valid 9-grids found" << std::endl;
                std::cout << "[DEBUG] Falling back to targeted algorithm as final approach" << std::endl;
            }
        }
    }
    
    // 如果分布式算法也失败，回退到原有的9宫格目标导向算法作为第三层
    if (config.enableNineConstraint && !config.nineColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] Trying 9-grid targeted algorithm as fallback" << std::endl;
        }
        
        auto nineRoutes = solveNineGridTargeted(config);
        if (!nineRoutes.empty()) {
            if (config.verbose) {
                std::cout << "[DEBUG] 9-grid targeted algorithm found " << nineRoutes.size() 
                         << " potential routes" << std::endl;
            }
            
            // 如果找到9宫格路径，根据verbose设置显示
            if (config.showRoutePath) {
                if (config.verbose) {
                    // Verbose mode: show all 9-grid routes
                    for (size_t i = 0; i < nineRoutes.size(); i++) {
                        std::cout << "9-Grid Route " << (i+1) << ": ";
                        nineRoutes[i].printRoute();
                    }
                } else {
                    // Default mode: show only first 9-grid route
                    if (!nineRoutes.empty()) {
                        nineRoutes[0].printRoute();
                    }
                }
            }
            return nineRoutes;
        } else {
            if (config.verbose) {
                std::cout << "[DEBUG] 9-grid targeted search completed but no valid 9-grids found" << std::endl;
                std::cout << "[DEBUG] 9FORCE mode requires strict 9-grid formation - no traditional search fallback" << std::endl;
            }
            
            std::cout << "\n[RESULT] 9FORCE Mode: Distributed clustering failed to form 9-grid." << std::endl;
            std::cout << "[ANALYSIS] Distributed algorithm analysis completed:" << std::endl;
            std::cout << "  - Distributed clustering: No clustering strategy succeeded" << std::endl;
            std::cout << "  - Targeted 9-grid search: No valid formation possible" << std::endl;
            std::cout << "[SUGGESTION] Try with enhanced parameters:" << std::endl;
            std::cout << "  pazusoba_v1.exe " << config.filePath << " 3 " << (config.maxStep + 30) << " " << (config.maxSize * 5) << " --nine-force=G --verbose" << std::endl;
            
            // 9FORCE模式严格要求，直接返回空结果，不进行传统搜索
            return std::vector<Route>();
        }
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

std::vector<Route> PSolver::solveNineGridTargeted(const SolverConfig& config) const
{
    std::vector<Route> routes;
    
    if (!config.enableNineConstraint || config.nineColors.empty())
    {
        if (config.verbose) {
            std::cout << "[DEBUG NineGridSolver] Nine-grid targeted solving not applicable" << std::endl;
        }
        return routes;
    }
    
    // 对每种目标颜色寻找9宫格
    for (const auto& targetColor : config.nineColors)
    {
        if (config.verbose) {
            std::cout << "[DEBUG NineGridSolver] Searching 9-grids for color " << (int)targetColor << std::endl;
        }
        
        auto nineTargets = findPossibleNineGrids(board, targetColor, config.verbose);
        
        if (nineTargets.empty())
        {
            if (config.verbose) {
                std::cout << "[DEBUG NineGridSolver] No feasible 9-grids found for color " << (int)targetColor << std::endl;
            }
            
            // 统计总的目标颜色珠子数
            int totalOrbs = 0;
            board.traverse([&](int i, int j, pad::orbs orb) {
                if (orb == targetColor) totalOrbs++;
            });
            
            if (totalOrbs >= 9) {
                std::cout << "\n[WARNING] 9FORCE Mode: Board has " << totalOrbs << " target orbs but they are scattered." << std::endl;
                std::cout << "[SUGGESTION] To form 9-grid, try increasing parameters:" << std::endl;
                std::cout << "  - Increase max steps: current=" << steps << ", try " << (steps + 10) << " or more" << std::endl;
                std::cout << "  - Increase search size: current=" << size << ", try " << (size * 2) << " or more" << std::endl;
                std::cout << "  - Example: pazusoba_v1.exe [board] 3 " << (steps + 10) << " " << (size * 2) << " --nine-force=G" << std::endl;
            } else {
                std::cout << "\n[ERROR] 9FORCE Mode: Board has only " << totalOrbs << " target orbs (need >=9 for 9-grid)" << std::endl;
                std::cout << "[SUGGESTION] This board cannot form a 9-grid. Try a different board or use regular mode." << std::endl;
            }
            
            return routes; // 返回空，9FORCE模式严格要求9宫格
        }
        
        // 选择最优的9宫格（已按combo数量和效率排序）
        const auto& bestTarget = nineTargets[0];
        if (config.verbose) {
            std::cout << "[DEBUG NineGridSolver] Selected BEST target at (" << bestTarget.centerX 
                     << "," << bestTarget.centerY << ") - Steps: " << bestTarget.estimatedSteps 
                     << ", Expected combos: " << bestTarget.expectedCombos 
                     << ", Efficiency: " << bestTarget.comboEfficiency << std::endl;
        }
        
        auto movePlan = planNineGridMoves(board, bestTarget, config.verbose);
        
        if (movePlan.empty())
        {
            if (config.verbose) {
                std::cout << "[DEBUG NineGridSolver] No moves needed for target - 9-grid already formed!" << std::endl;
            }
            // 创建一个空的路径表示9宫格已经存在
            continue;
        }
        
        // 尝试生成实际的路径
        auto generatedRoutes = generateNineGridRoutes(bestTarget, movePlan, config);
        if (!generatedRoutes.empty()) {
            if (config.verbose) {
                std::cout << "[DEBUG NineGridSolver] Successfully generated " << generatedRoutes.size() 
                         << " routes using 9-grid targeted approach" << std::endl;
            }
            return generatedRoutes;
        } else {
            // 如果路径生成失败，提供目标引导信息给传统搜索
            if (config.verbose) {
                std::cout << "[DEBUG NineGridSolver] 9-grid target found: (" << bestTarget.centerX 
                         << "," << bestTarget.centerY << ") with " << bestTarget.expectedCombos 
                         << " expected combos" << std::endl;
                std::cout << "[DEBUG NineGridSolver] Target requires " << movePlan.size() 
                         << " moves, estimated " << bestTarget.estimatedSteps << " steps total" << std::endl;
                std::cout << "[DEBUG NineGridSolver] Route generation failed, continuing with traditional search" << std::endl;
            }
        }
        
        // 由于Route类需要PState构造，我们暂时返回空列表
        // 让算法回退到传统搜索
        return routes;
    }
    
    return routes;
}

// ===== 9宫格路径生成辅助方法实现 =====

std::vector<Route> PSolver::generateNineGridRoutes(const NineTarget& target, const std::vector<OrbMoveplan>& movePlan, const SolverConfig& config) const
{
    std::vector<Route> routes;
    
    if (config.verbose) {
        std::cout << "[DEBUG NineGridSolver] 9FORCE mode: No extended search - returning empty routes" << std::endl;
    }
    
    // 在9FORCE模式下，如果简单移动计划不能直接形成9宫格，就直接返回失败
    // 不进行扩展搜索，因为那实际上就是传统搜索的一种形式
    
    return routes; // 直接返回空，不做任何搜索
}

PState* PSolver::buildOptimalNineGridState(const NineTarget& target, const std::vector<OrbMoveplan>& movePlan) const
{
    // 简化实现：创建一个基础状态用于演示
    // 在实际应用中，这需要更复杂的路径规划算法
    
    if (movePlan.empty()) {
        return nullptr;
    }
    
    // 选择第一个移动作为起点
    const auto& firstMove = movePlan[0];
    OrbLocation startLoc(firstMove.fromX, firstMove.fromY);
    OrbLocation targetLoc(firstMove.toX, firstMove.toY);
    
    // 创建一个简单的单步移动状态
    auto state = new PState(board, startLoc, targetLoc, 1, steps);
    
    return state;
}

std::vector<std::pair<int, int>> PSolver::calculateOptimalMoveSequence(const std::vector<OrbMoveplan>& movePlan) const
{
    std::vector<std::pair<int, int>> sequence;
    
    // 简化为按优先级选择移动
    for (const auto& move : movePlan) {
        sequence.push_back({move.fromX, move.fromY});
    }
    
    return sequence;
}

bool PSolver::validateNineGridFormation(const Route& route, pad::orbs targetColor, bool verbose) const
{
    // 获取最终棋盘状态
    std::string finalBoardStr = route.getFinalBoardString();
    
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

// ===== 分布式聚集算法实现 =====

std::vector<Route> PSolver::solveNineGridDistributed(const SolverConfig& config) const
{
    if (config.verbose) {
        std::cout << "[DEBUG Distributed] ========== DISTRIBUTED CLUSTERING ENTRY POINT ==========" << std::endl;
    }
    
    std::vector<Route> routes;
    
    if (!config.enableNineConstraint || config.nineColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG Distributed] Distributed nine-grid solving not applicable - constraint disabled or no colors" << std::endl;
        }
        return routes;
    }
    
    if (config.verbose) {
        std::cout << "[DEBUG Distributed] Starting distributed clustering algorithm for 9-grid formation" << std::endl;
        std::cout << "[DEBUG Distributed] Target colors: " << config.nineColors.size() << " colors specified" << std::endl;
    }
    
    // 对每种目标颜色尝试分布式算法
    for (const auto& targetColor : config.nineColors) {
        if (config.verbose) {
            std::cout << "[DEBUG Distributed] Distributed approach for color " << (int)targetColor << std::endl;
        }
        
        // 寻找最优的9宫格目标
        auto nineTargets = findPossibleNineGrids(board, targetColor, config.verbose);
        
        if (nineTargets.empty()) {
            if (config.verbose) {
                std::cout << "[DEBUG Distributed] No feasible 9-grid targets for distributed approach" << std::endl;
            }
            continue;
        }
        
        // 选择最优目标（已按效率排序）
        const auto& bestTarget = nineTargets[0];
        if (config.verbose) {
            std::cout << "[DEBUG Distributed] Selected target at (" << bestTarget.centerX 
                     << "," << bestTarget.centerY << ") for distributed approach" << std::endl;
        }
        
        // 第一阶段：珠子聚集
        auto gatherPath = phaseOneGatherOrbs(bestTarget, config.verbose);
        
        if (!gatherPath.empty()) {
            if (config.verbose) {
                std::cout << "[DEBUG Distributed] Phase 1 completed: gathered orbs in " 
                         << gatherPath.size() << " steps" << std::endl;
            }
            
            // 模拟第一阶段后的棋盘状态
            PBoard clusteredBoard = board;
            for (size_t i = 1; i < gatherPath.size(); i++) {
                OrbLocation from(gatherPath[i-1].first, gatherPath[i-1].second);
                OrbLocation to(gatherPath[i].first, gatherPath[i].second);
                clusteredBoard.swapLocation(from, to);
            }
            
            // 第二阶段：精确排列，从阶段1终点开始
            std::pair<int, int> phase1EndPos = gatherPath.empty() ? std::make_pair(bestTarget.centerX, bestTarget.centerY) : gatherPath.back();
            auto arrangePath = phaseTwoArrangeGrid(clusteredBoard, bestTarget, phase1EndPos, config.verbose);
            
            if (!arrangePath.empty()) {
                // 智能合并两个阶段的路径，避免重复连接点
                std::vector<std::pair<int, int>> completePath = gatherPath;
                
                if (!arrangePath.empty()) {
                    // 检查gatherPath的终点是否与arrangePath的起点相同
                    bool needsConnection = true;
                    if (!gatherPath.empty() && !arrangePath.empty()) {
                        auto gatherEnd = gatherPath.back();
                        auto arrangeStart = arrangePath[0];
                        if (gatherEnd.first == arrangeStart.first && gatherEnd.second == arrangeStart.second) {
                            // 终点和起点相同，跳过arrangePath的第一个点
                            completePath.insert(completePath.end(), arrangePath.begin() + 1, arrangePath.end());
                            needsConnection = false;
                        }
                    }
                    
                    if (needsConnection) {
                        // 没有重复，直接连接所有arrangePath的点
                        completePath.insert(completePath.end(), arrangePath.begin(), arrangePath.end());
                    }
                }
                
                if (config.verbose) {
                    std::cout << "[DEBUG Distributed] Phase 2 completed: arranged grid in " 
                             << arrangePath.size() << " additional steps" << std::endl;
                    std::cout << "[DEBUG Distributed] Total distributed path: " 
                             << completePath.size() << " steps" << std::endl;
                }
                
                // 将路径转换为Route对象
                Route* distributedRoute = convertDistributedPathToRoute(completePath, bestTarget, config);
                if (distributedRoute != nullptr) {
                    routes.push_back(*distributedRoute);
                    delete distributedRoute;
                    
                    if (config.verbose) {
                        std::cout << "[DEBUG Distributed] Complete distributed path: ";
                        for (size_t i = 0; i < completePath.size(); i++) {
                            std::cout << "(" << completePath[i].first << "," << completePath[i].second << ")";
                            if (i < completePath.size() - 1) std::cout << " -> ";
                        }
                        std::cout << std::endl;
                        
                        std::cout << "[DEBUG Distributed] Final board from route:" << std::endl;
                        std::cout << routes.back().getFinalBoardStringMultiLine() << std::endl;
                    }
                    
                    // 验证路径确实能形成9宫格
                    if (validateNineGridFormation(routes.back(), targetColor, config.verbose)) {
                        if (config.verbose) {
                            std::cout << "[DEBUG Distributed] Distributed path validated - forms valid 9-grid!" << std::endl;
                        }
                        return routes; // 成功找到解决方案
                    } else {
                        if (config.verbose) {
                            std::cout << "[DEBUG Distributed] Distributed path validation failed" << std::endl;
                        }
                        routes.pop_back();
                    }
                }
            } else {
                if (config.verbose) {
                    std::cout << "[DEBUG Distributed] Phase 2 failed: could not arrange clustered orbs into grid" << std::endl;
                }
            }
        } else {
            if (config.verbose) {
                std::cout << "[DEBUG Distributed] Phase 1 failed: could not gather orbs effectively" << std::endl;
            }
        }
    }
    
    return routes;
}

std::vector<std::pair<int, int>> PSolver::phaseOneGatherOrbs(const NineTarget& target, bool verbose) const
{
    if (verbose) {
        std::cout << "[DEBUG Distributed Phase1] Gathering " << (int)target.targetColor 
                 << " orbs near target region (" << target.centerX << "," << target.centerY << ")" << std::endl;
    }
    
    std::vector<std::pair<int, int>> gatherPath;
    PBoard currentBoard = board;
    
    // 定义聚集区域（5x5区域，以目标9宫格为中心）
    int clusterMinX = std::max(0, target.centerX - 2);
    int clusterMaxX = std::min(row - 1, target.centerX + 2);
    int clusterMinY = std::max(0, target.centerY - 2);
    int clusterMaxY = std::min(column - 1, target.centerY + 2);
    
    // 找到所有目标颜色珠子
    std::vector<std::pair<int, int>> targetOrbs;
    currentBoard.traverse([&](int i, int j, pad::orbs orb) {
        if (orb == target.targetColor) {
            targetOrbs.push_back({i, j});
        }
    });
    
    if (verbose) {
        std::cout << "[DEBUG Distributed Phase1] Found " << targetOrbs.size() 
                 << " target orbs, cluster area: (" << clusterMinX << "," << clusterMinY 
                 << ") to (" << clusterMaxX << "," << clusterMaxY << ")" << std::endl;
    }
    
    // 计算有多少珠子已经在聚集区域内
    int orbsInCluster = 0;
    for (const auto& orbPos : targetOrbs) {
        if (orbPos.first >= clusterMinX && orbPos.first <= clusterMaxX &&
            orbPos.second >= clusterMinY && orbPos.second <= clusterMaxY) {
            orbsInCluster++;
        }
    }
    
    if (verbose) {
        std::cout << "[DEBUG Distributed Phase1] " << orbsInCluster 
                 << " orbs already in cluster area, need to move " << (9 - orbsInCluster) << " more" << std::endl;
    }
    
    // 如果已经有足够珠子在聚集区域，直接返回简单路径
    if (orbsInCluster >= 9) {
        if (verbose) {
            std::cout << "[DEBUG Distributed Phase1] Sufficient orbs already clustered, minimal gathering needed" << std::endl;
        }
        // 返回一个简单的单步路径作为Phase1的象征性完成
        if (!targetOrbs.empty()) {
            gatherPath.push_back(targetOrbs[0]);
            gatherPath.push_back({targetOrbs[0].first, targetOrbs[0].second}); // 原地不动
        }
        return gatherPath;
    }
    
    // 按距离聚集区域的远近排序（远的先移动）
    std::sort(targetOrbs.begin(), targetOrbs.end(), [&](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        int distA = abs(a.first - target.centerX) + abs(a.second - target.centerY);
        int distB = abs(b.first - target.centerX) + abs(b.second - target.centerY);
        return distA > distB; // 距离远的优先处理
    });
    
    int orbsNeeded = 9 - orbsInCluster;
    int orbsMoved = 0;
    
    for (const auto& orbPos : targetOrbs) {
        if (orbsMoved >= orbsNeeded) break;
        
        // 检查是否已经在聚集区域内
        if (orbPos.first >= clusterMinX && orbPos.first <= clusterMaxX &&
            orbPos.second >= clusterMinY && orbPos.second <= clusterMaxY) {
            continue; // 已经在目标区域，跳过
        }
        
        // 简单移动策略：朝目标区域中心移动
        auto movePath = moveOrbToCluster(currentBoard, orbPos.first, orbPos.second, target);
        
        if (!movePath.empty() && movePath.size() > 1) { // 至少要有移动
            // 添加到总路径
            if (gatherPath.empty()) {
                gatherPath = movePath;
            } else {
                // 连接路径，从上一个路径的终点开始
                auto lastPos = gatherPath.back();
                gatherPath.insert(gatherPath.end(), movePath.begin() + 1, movePath.end());
            }
            
            // 模拟移动，更新棋盘状态
            for (size_t j = 1; j < movePath.size(); j++) {
                OrbLocation from(movePath[j-1].first, movePath[j-1].second);
                OrbLocation to(movePath[j].first, movePath[j].second);
                currentBoard.swapLocation(from, to);
            }
            
            orbsMoved++;
            
            if (verbose) {
                std::cout << "[DEBUG Distributed Phase1] Moved orb #" << orbsMoved 
                         << " from (" << orbPos.first << "," << orbPos.second 
                         << ") in " << (movePath.size()-1) << " steps" << std::endl;
            }
        }
        
        // 限制路径长度，避免过度复杂
        if (gatherPath.size() > 25) {
            if (verbose) {
                std::cout << "[DEBUG Distributed Phase1] Path length limit reached, stopping at " 
                         << gatherPath.size() << " steps" << std::endl;
            }
            break;
        }
    }
    
    if (verbose) {
        std::cout << "[DEBUG Distributed Phase1] Gathering completed: " << orbsMoved 
                 << " orbs moved, total path length: " << gatherPath.size() << std::endl;
    }
    
    return gatherPath;
}

std::vector<std::pair<int, int>> PSolver::moveOrbToCluster(const PBoard& board, int fromX, int fromY, const NineTarget& target) const
{
    std::vector<std::pair<int, int>> path;
    path.push_back({fromX, fromY});
    
    int currentX = fromX, currentY = fromY;
    int maxMoves = 5; // 限制单个珠子的最大移动步数
    
    for (int move = 0; move < maxMoves; move++) {
        // 计算朝目标中心的方向
        int deltaX = 0, deltaY = 0;
        
        if (currentX < target.centerX) deltaX = 1;
        else if (currentX > target.centerX) deltaX = -1;
        
        if (currentY < target.centerY) deltaY = 1;
        else if (currentY > target.centerY) deltaY = -1;
        
        // 如果已经很接近目标区域，停止移动
        if (abs(currentX - target.centerX) <= 2 && abs(currentY - target.centerY) <= 2) {
            break;
        }
        
        // 尝试移动
        int newX = currentX + deltaX;
        int newY = currentY + deltaY;
        
        // 检查边界
        if (newX >= 0 && newX < row && newY >= 0 && newY < column) {
            path.push_back({newX, newY});
            currentX = newX;
            currentY = newY;
        } else {
            break; // 无法继续移动
        }
    }
    
    return path;
}

// 创建从源位置到目标位置的精确移动序列
std::vector<std::pair<int, int>> PSolver::createPreciseMoveSequence(const std::pair<int, int>& from, const std::pair<int, int>& to, const PBoard& board) const
{
    std::vector<std::pair<int, int>> sequence;
    sequence.push_back(from);
    
    int currentX = from.first, currentY = from.second;
    int targetX = to.first, targetY = to.second;
    int maxMoves = 10; // 防止无限循环
    
    // 使用简单的贪心策略：每次朝目标方向移动一步
    for (int move = 0; move < maxMoves && (currentX != targetX || currentY != targetY); move++) {
        int deltaX = 0, deltaY = 0;
        
        // 计算朝目标的方向
        if (currentX < targetX) deltaX = 1;
        else if (currentX > targetX) deltaX = -1;
        
        if (currentY < targetY) deltaY = 1;
        else if (currentY > targetY) deltaY = -1;
        
        // 尝试移动
        int newX = currentX + deltaX;
        int newY = currentY + deltaY;
        
        // 检查边界
        if (newX >= 0 && newX < row && newY >= 0 && newY < column) {
            sequence.push_back({newX, newY});
            currentX = newX;
            currentY = newY;
        } else {
            // 如果直线路径不可行，尝试先水平或垂直移动
            if (deltaX != 0 && currentY + deltaY >= 0 && currentY + deltaY < column) {
                sequence.push_back({currentX, currentY + deltaY});
                currentY = currentY + deltaY;
            } else if (deltaY != 0 && currentX + deltaX >= 0 && currentX + deltaX < row) {
                sequence.push_back({currentX + deltaX, currentY});
                currentX = currentX + deltaX;
            } else {
                break; // 无法继续移动
            }
        }
    }
    
    return sequence;
}

std::vector<std::pair<int, int>> PSolver::phaseTwoArrangeGrid(const PBoard& clusteredBoard, const NineTarget& target, const std::pair<int, int>& phase1EndPos, bool verbose) const
{
    if (verbose) {
        std::cout << "[DEBUG Distributed Phase2] 8+1 Strategy: Arranging 8 orbs, leaving 1 space for moving orb" << std::endl;
        std::cout << "[DEBUG Distributed Phase2] Target 3x3 grid center at (" 
                 << target.centerX << "," << target.centerY << ")" << std::endl;
    }
    
    std::vector<std::pair<int, int>> arrangePath;
    
    // 生成目标9宫格的所有9个位置
    std::vector<std::pair<int, int>> nineGridPositions;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int x = target.centerX + dx;
            int y = target.centerY + dy;
            if (x >= 0 && x < row && y >= 0 && y < column) {
                nineGridPositions.push_back({x, y});
            }
        }
    }
    
    if (nineGridPositions.size() != 9) {
        if (verbose) {
            std::cout << "[DEBUG Distributed Phase2] Invalid grid - not all 9 positions are within bounds" << std::endl;
        }
        return {};
    }
    
    // 找到最适合作为"移动珠子"的目标颜色珠子（在9宫格内或最近）
    int bestMovingOrbX = -1, bestMovingOrbY = -1;
    int minDistanceToGrid = 999;
    
    // 扫描聚集区域寻找目标颜色珠子
    int clusterMinX = std::max(0, target.centerX - 2);
    int clusterMaxX = std::min(row - 1, target.centerX + 2);
    int clusterMinY = std::max(0, target.centerY - 2);
    int clusterMaxY = std::min(column - 1, target.centerY + 2);
    
    for (int i = clusterMinX; i <= clusterMaxX; i++) {
        for (int j = clusterMinY; j <= clusterMaxY; j++) {
            if (getOrbAt(clusteredBoard, i, j) == target.targetColor) {
                int distanceToCenter = abs(i - target.centerX) + abs(j - target.centerY);
                if (distanceToCenter < minDistanceToGrid) {
                    minDistanceToGrid = distanceToCenter;
                    bestMovingOrbX = i;
                    bestMovingOrbY = j;
                }
            }
        }
    }
    
    if (bestMovingOrbX == -1) {
        if (verbose) {
            std::cout << "[DEBUG Distributed Phase2] No suitable moving orb found" << std::endl;
        }
        return {};
    }
    
    if (verbose) {
        std::cout << "[DEBUG Distributed Phase2] Selected moving orb at (" 
                 << bestMovingOrbX << "," << bestMovingOrbY << ") distance=" << minDistanceToGrid << std::endl;
    }
    
    // 8+1策略：为其他8个位置找到目标珠子，留下1个位置给移动珠子
    std::vector<std::pair<int, int>> targetPositions;
    std::pair<int, int> movingOrbFinalPos;
    
    // 分配位置：选择距离移动珠子最远的位置作为其最终位置
    int maxDistance = -1;
    for (const auto& pos : nineGridPositions) {
        int distance = abs(pos.first - bestMovingOrbX) + abs(pos.second - bestMovingOrbY);
        if (distance > maxDistance) {
            maxDistance = distance;
            movingOrbFinalPos = pos;
        }
    }
    
    // 其他8个位置作为需要填充的位置
    for (const auto& pos : nineGridPositions) {
        if (pos.first != movingOrbFinalPos.first || pos.second != movingOrbFinalPos.second) {
            // 检查这个位置是否已经有目标颜色珠子
            if (getOrbAt(clusteredBoard, pos.first, pos.second) != target.targetColor) {
                targetPositions.push_back(pos);
            }
        }
    }
    
    if (verbose) {
        std::cout << "[DEBUG Distributed Phase2] Need to fill " << targetPositions.size() 
                 << " positions, moving orb final position: (" 
                 << movingOrbFinalPos.first << "," << movingOrbFinalPos.second << ")" << std::endl;
    }
    
    // 找到可用的目标颜色珠子来填充这些位置
    std::vector<std::pair<int, int>> availableOrbs;
    for (int i = clusterMinX; i <= clusterMaxX; i++) {
        for (int j = clusterMinY; j <= clusterMaxY; j++) {
            if (getOrbAt(clusteredBoard, i, j) == target.targetColor) {
                // 跳过我们选定的移动珠子
                if (i == bestMovingOrbX && j == bestMovingOrbY) continue;
                
                // 跳过已经在正确位置的珠子
                bool alreadyInCorrectPosition = false;
                for (const auto& gridPos : nineGridPositions) {
                    if (i == gridPos.first && j == gridPos.second) {
                        if (getOrbAt(clusteredBoard, i, j) == target.targetColor) {
                            alreadyInCorrectPosition = true;
                            break;
                        }
                    }
                }
                
                if (!alreadyInCorrectPosition) {
                    availableOrbs.push_back({i, j});
                }
            }
        }
    }
    
    if (verbose) {
        std::cout << "[DEBUG Distributed Phase2] Found " << availableOrbs.size() 
                 << " available orbs for filling positions" << std::endl;
    }
    
    // 8+1策略：创建连续的移动路径，从阶段1的终点开始
    if (targetPositions.size() <= availableOrbs.size()) {
        // 从阶段1的终点开始连续移动
        std::pair<int, int> currentPos = phase1EndPos;
        
        if (arrangePath.empty()) {
            arrangePath.push_back(currentPos);
        }
        
        if (verbose) {
            std::cout << "[DEBUG Distributed Phase2] 8+1 Strategy: Starting from phase1 end position (" 
                     << currentPos.first << "," << currentPos.second << ")" << std::endl;
            std::cout << "[DEBUG Distributed Phase2] Moving orb selected at (" 
                     << bestMovingOrbX << "," << bestMovingOrbY << ") will end at (" 
                     << movingOrbFinalPos.first << "," << movingOrbFinalPos.second << ")" << std::endl;
        }
        
        // 按优先级处理需要填充的位置
        for (size_t i = 0; i < targetPositions.size() && i < availableOrbs.size(); i++) {
            auto targetPos = targetPositions[i];
            auto orbPos = availableOrbs[i];
            
            if (verbose) {
                std::cout << "[DEBUG Distributed Phase2] 8+1 Step " << (i+1) << ": bring orb from (" 
                         << orbPos.first << "," << orbPos.second << ") to (" 
                         << targetPos.first << "," << targetPos.second << ")" << std::endl;
            }
            
            // 创建从当前位置到珠子位置，再到目标位置的路径
            auto pathToOrb = createPreciseMoveSequence(currentPos, orbPos, clusteredBoard);
            auto pathToTarget = createPreciseMoveSequence(orbPos, targetPos, clusteredBoard);
            
            // 添加到珠子位置的路径（跳过起始点避免重复）
            if (pathToOrb.size() > 1) {
                arrangePath.insert(arrangePath.end(), pathToOrb.begin() + 1, pathToOrb.end());
                currentPos = pathToOrb.back();
            }
            
            // 添加从珠子到目标位置的路径（跳过起始点避免重复）
            if (pathToTarget.size() > 1) {
                arrangePath.insert(arrangePath.end(), pathToTarget.begin() + 1, pathToTarget.end());
                currentPos = pathToTarget.back();
            }
        }
        
        // 最后移动到最终位置
        auto finalPath = createPreciseMoveSequence(currentPos, movingOrbFinalPos, clusteredBoard);
        if (finalPath.size() > 1) {
            arrangePath.insert(arrangePath.end(), finalPath.begin() + 1, finalPath.end());
        }
        
        if (verbose) {
            std::cout << "[DEBUG Distributed Phase2] Final step: move to final position (" 
                     << movingOrbFinalPos.first << "," << movingOrbFinalPos.second << ")" << std::endl;
            std::cout << "[DEBUG Distributed Phase2] 8+1 Strategy completed with " 
                     << arrangePath.size() << " total steps" << std::endl;
        }
        
        return arrangePath;
    } else {
        if (verbose) {
            std::cout << "[DEBUG Distributed Phase2] Insufficient available orbs (" 
                     << availableOrbs.size() << ") for target positions (" 
                     << targetPositions.size() << ")" << std::endl;
        }
        return {}; // 8+1策略失败
    }
}

// 分布式路径转换为Route的辅助方法
Route* PSolver::convertDistributedPathToRoute(const std::vector<std::pair<int, int>>& distributedPath, 
                                              const NineTarget& target, 
                                              const SolverConfig& config) const
{
    if (config.verbose) {
        std::cout << "[DEBUG convertDistributedPath] Input path size: " << distributedPath.size() << std::endl;
        if (!distributedPath.empty()) {
            std::cout << "[DEBUG convertDistributedPath] Path: ";
            for (size_t i = 0; i < distributedPath.size(); i++) {
                std::cout << "(" << distributedPath[i].first << "," << distributedPath[i].second << ")";
                if (i < distributedPath.size() - 1) std::cout << " -> ";
            }
            std::cout << std::endl;
        }
    }
    
    if (distributedPath.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG convertDistributedPath] Empty path, returning nullopt" << std::endl;
        }
        return nullptr;
    }
    
    // 创建路径的PState链 - 确保起始珠子是正确的目标颜色
    std::vector<PState*> states;
    PBoard currentBoard = board;
    
    // 验证起始位置是否包含目标颜色珠子
    OrbLocation startPos(distributedPath[0].first, distributedPath[0].second);
    pad::orbs startOrb = getOrbAt(currentBoard, startPos.first, startPos.second);
    
    if (config.verbose) {
        std::cout << "[DEBUG convertDistributedPath] Starting position (" << startPos.first 
                 << "," << startPos.second << ") contains orb type: " << (int)startOrb
                 << ", target color: " << (int)target.targetColor << std::endl;
    }
    
    if (startOrb != target.targetColor) {
        if (config.verbose) {
            std::cout << "[DEBUG convertDistributedPath] ERROR: Starting position does not contain target color orb!" << std::endl;
            std::cout << "[DEBUG convertDistributedPath] Path planning failed - starting orb mismatch" << std::endl;
        }
        return nullptr;
    }
    
    // 起始状态
    auto rootState = new PState(currentBoard, startPos, startPos, 0, steps);
    states.push_back(rootState);
    
    // 逐步创建状态链，并验证每一步移动的珠子颜色
    for (size_t i = 1; i < distributedPath.size(); i++) {
        OrbLocation fromPos(distributedPath[i-1].first, distributedPath[i-1].second);
        OrbLocation toPos(distributedPath[i].first, distributedPath[i].second);
        
        // 验证当前要移动的珠子是否为目标颜色
        pad::orbs currentOrb = getOrbAt(currentBoard, fromPos.first, fromPos.second);
        if (currentOrb != target.targetColor) {
            if (config.verbose) {
                std::cout << "[DEBUG convertDistributedPath] WARNING: Step " << i 
                         << " trying to move non-target orb (type " << (int)currentOrb 
                         << ") from (" << fromPos.first << "," << fromPos.second << ")" << std::endl;
            }
        }
        
        auto nextState = new PState(currentBoard, fromPos, toPos, static_cast<int>(i), steps);
        nextState->parent = states.back();
        states.push_back(nextState);
        
        // 更新棋盘状态
        currentBoard.swapLocation(fromPos, toPos);
        
        if (config.verbose) {
            std::cout << "[DEBUG convertDistributedPath] Step " << i << ": moved orb from (" 
                     << fromPos.first << "," << fromPos.second << ") to (" 
                     << toPos.first << "," << toPos.second << ")" << std::endl;
        }
    }
    
    // 使用最终状态创建Route
    if (!states.empty()) {
        Route route(states.back());
        
        if (config.verbose) {
            std::cout << "[DEBUG convertDistributedPath] Successfully converted to Route" << std::endl;
            
            // 打印珠子的最终停止位置
            std::pair<int, int> finalStopPosition = distributedPath.back();
            std::cout << "[DEBUG convertDistributedPath] *** FINAL STOP POSITION: (" 
                     << finalStopPosition.first << "," << finalStopPosition.second << ") ***" << std::endl;
            
            // 打印目标9宫格的所有位置及其当前珠子类型
            std::cout << "[DEBUG convertDistributedPath] Target 9-grid positions around center (" 
                     << target.centerX << "," << target.centerY << "):" << std::endl;
            
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int x = target.centerX + dx;
                    int y = target.centerY + dy;
                    if (x >= 0 && x < row && y >= 0 && y < column) {
                        pad::orbs orbAtPos = getOrbAt(currentBoard, x, y);
                        char orbChar = (orbAtPos == pad::fire) ? 'R' : 
                                      (orbAtPos == pad::water) ? 'B' :
                                      (orbAtPos == pad::wood) ? 'G' :
                                      (orbAtPos == pad::light) ? 'L' :
                                      (orbAtPos == pad::dark) ? 'D' :
                                      (orbAtPos == pad::recovery) ? 'H' : '?';
                        std::cout << "[DEBUG convertDistributedPath]   Position (" << x << "," << y 
                                 << ") = " << orbChar << " (target: " 
                                 << ((target.targetColor == orbAtPos) ? "MATCH" : "MISS") << ")" << std::endl;
                    }
                }
            }
        }
        
        // 清理状态内存（Route会复制需要的数据）
        for (auto* state : states) {
            delete state;
        }
        
        return new Route(route);
    }
    
    if (config.verbose) {
        std::cout << "[DEBUG convertDistributedPath] Failed to convert to Route (states empty)" << std::endl;
    }
    
    return nullptr;
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

void PSolver::setStepLimit(int step) { this->steps = step; }
