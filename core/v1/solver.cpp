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
    // Default profile (always include ComboProfile)
    std::vector<Profile*> profiles;
    profiles.push_back(new ComboProfile);
    
    // Add default NineProfile for wood (keeping original behavior)
    profiles.push_back(new NineProfile({pad::wood}));
    
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
    } else {
        // Keep default wood nine profile if no custom nine profile and no forced mode
        if (!config.enableNineConstraint) {
            profiles.push_back(new NineProfile({pad::wood}));
        }
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

    // 如果是9FORCE模式，优先尝试9宫格目标导向算法
    if (config.enableNineConstraint && !config.nineColors.empty()) {
        if (config.verbose) {
            std::cout << "[DEBUG] 9FORCE mode detected, trying 9-grid targeted algorithm first" << std::endl;
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
                std::cout << "[DEBUG] 9-grid targeted algorithm analysis completed, continuing with traditional search" << std::endl;
                std::cout << "[DEBUG] Traditional search will now be guided by 9FORCE penalties toward 9-grid formation" << std::endl;
            }
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
    
    // 判断是否可能形成9宫格：需要至少9个目标颜色珠子
    return availableOrbs >= 9;
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
                std::cout << "[DEBUG NineGridSolver] No possible 9-grids found for color " << (int)targetColor << std::endl;
            }
            continue;
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
        
        // 暂时返回空路径列表，表示找到了可行的9宫格目标
        // 实际的路径计算需要更复杂的实现
        if (config.verbose) {
            std::cout << "[DEBUG NineGridSolver] 9-grid target found but route generation not fully implemented" << std::endl;
            std::cout << "[DEBUG NineGridSolver] Planned " << movePlan.size() << " moves to form 9-grid" << std::endl;
        }
        
        // 由于Route类需要PState构造，我们暂时返回空列表
        // 让算法回退到传统搜索
        return routes;
    }
    
    return routes;
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
