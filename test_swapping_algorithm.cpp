#include <iostream>
#include <vector>
#include <cmath>

struct Pair {
    int x, y;
    Pair(int x, int y) : x(x), y(y) {}
};

// 简化版的 swapping-optimized 算法测试
std::vector<Pair> buildSwappingOptimizedPathTest(int centerX, int centerY, int row, int column) {
    std::vector<Pair> path;
    
    // 定义3x3目标区域
    int minX = std::max(0, centerX - 1);
    int maxX = std::min(row - 1, centerX + 1);
    int minY = std::max(0, centerY - 1);
    int maxY = std::min(column - 1, centerY + 1);
    
    std::cout << "Target 3x3 area: (" << minX << "," << minY << ") to (" << maxX << "," << maxY << ")" << std::endl;
    
    // 策略1：创建蛇形路径覆盖整个棋盘
    int currentX = 0, currentY = 0;
    path.push_back(Pair(currentX, currentY));
    
    // 创建蛇形路径
    while (currentX < row) {
        // 向右移动
        while (currentY < column) {
            if (currentX != 0 || currentY != 0) {
                path.push_back(Pair(currentX, currentY));
            }
            currentY++;
        }
        currentY--;
        
        if (currentX + 1 < row) {
            currentX++;
            path.push_back(Pair(currentX, currentY));
            
            // 向左移动
            while (currentY > 0) {
                currentY--;
                path.push_back(Pair(currentX, currentY));
            }
            
            if (currentX + 1 < row) {
                currentX++;
                path.push_back(Pair(currentX, currentY));
            }
        } else {
            break;
        }
        
        if (path.size() > 30) break;
    }
    
    // 策略2：在目标区域周围创建漩涡效应
    std::vector<Pair> vortexPath;
    for (int radius = 1; radius <= 3; radius++) {
        for (int angle = 0; angle < 360; angle += 30) {
            double rad = angle * 3.14159265 / 180.0;
            int x = centerX + (int)(radius * cos(rad));
            int y = centerY + (int)(radius * sin(rad));
            
            x = std::max(0, std::min(row - 1, x));
            y = std::max(0, std::min(column - 1, y));
            
            vortexPath.push_back(Pair(x, y));
        }
    }
    
    path.insert(path.end(), vortexPath.begin(), vortexPath.end());
    
    // 策略3：目标区域内精细调整
    for (int pass = 0; pass < 2; pass++) {
        for (int i = minX; i <= maxX; i++) {
            for (int j = minY; j <= maxY; j++) {
                if ((i + j + pass) % 2 == 0) {
                    path.push_back(Pair(i, j));
                }
            }
        }
    }
    
    return path;
}

void printPath(const std::vector<Pair>& path) {
    std::cout << "Path with " << path.size() << " steps:" << std::endl;
    for (size_t i = 0; i < std::min(path.size(), size_t(20)); i++) {
        std::cout << "(" << path[i].x << "," << path[i].y << ") ";
        if ((i + 1) % 10 == 0) std::cout << std::endl;
    }
    if (path.size() > 20) {
        std::cout << "... (showing first 20 steps)" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Testing Swapping-Optimized Algorithm ===" << std::endl;
    
    // 测试用例：目标在 (2,3)，棋盘 5x6
    int centerX = 2, centerY = 3;
    int row = 5, column = 6;
    
    auto path = buildSwappingOptimizedPathTest(centerX, centerY, row, column);
    printPath(path);
    
    std::cout << "\n=== Algorithm Analysis ===" << std::endl;
    std::cout << "1. Snake pattern: Covers entire board systematically" << std::endl;
    std::cout << "2. Vortex pattern: Creates swirling motion around target" << std::endl;
    std::cout << "3. Fine-tuning: Precise movements within target area" << std::endl;
    std::cout << "\nKey insights:" << std::endl;
    std::cout << "- Each step swaps current orb with adjacent orb" << std::endl;
    std::cout << "- Path design embraces swapping mechanism" << std::endl;
    std::cout << "- Multiple passes gradually shuffle orbs into position" << std::endl;
    std::cout << "- Systematic coverage ensures all orbs participate in swapping" << std::endl;
    
    return 0;
}