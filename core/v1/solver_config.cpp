/**
 * solver_config.cpp
 * Extended configuration for pazusoba solver
 * by Yiheng Quan
 */

#include "solver_config.h"
#include <iostream>
#include <algorithm>

pad::orbs SolverConfig::parseColor(const std::string& colorStr)
{
    std::string lower = colorStr;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "r" || lower == "red" || lower == "fire") return pad::fire;
    if (lower == "b" || lower == "blue" || lower == "water") return pad::water;
    if (lower == "g" || lower == "green" || lower == "wood") return pad::wood;
    if (lower == "l" || lower == "light" || lower == "yellow") return pad::light;
    if (lower == "d" || lower == "dark" || lower == "purple") return pad::dark;
    if (lower == "h" || lower == "heal" || lower == "recovery") return pad::recovery;
    
    return pad::unknown;
}

std::string SolverConfig::colorToString(pad::orbs color)
{
    switch (color) {
        case pad::fire: return "R(Fire)";
        case pad::water: return "B(Water)";
        case pad::wood: return "G(Wood)";
        case pad::light: return "L(Light)";
        case pad::dark: return "D(Dark)";
        case pad::recovery: return "H(Heal)";
        default: return "Unknown";
    }
}

std::vector<pad::orbs> SolverConfig::parseColorList(const std::string& colorStr)
{
    std::vector<pad::orbs> colors;
    
    for (char c : colorStr) {
        std::string singleChar(1, c);
        pad::orbs color = parseColor(singleChar);
        if (color != pad::unknown) {
            colors.push_back(color);
        }
    }
    
    return colors;
}

void SolverConfig::printUsage()
{
    std::cout << "\nPazusoba Solver V1 - Extended Usage\n";
    std::cout << "==================================\n\n";
    
    std::cout << "Usage: pazusoba_v1.exe [board] [minErase] [maxStep] [maxSize] [options]\n\n";
    
    std::cout << "Basic Parameters:\n";
    std::cout << "  board      - Board string or file path\n";
    std::cout << "  minErase   - Minimum orbs to erase (default: 3)\n";
    std::cout << "  maxStep    - Maximum steps (default: 30)\n";
    std::cout << "  maxSize    - Search size (default: 20000)\n\n";
    
    std::cout << "Extended Options:\n";
    std::cout << "  --colors=COLORS    - Priority colors (e.g., RBG for Red,Blue,Green)\n";
    std::cout << "  --plus=COLORS      - Enable plus(+) priority for colors\n";
    std::cout << "  --nine=COLORS      - Enable 9-grid priority for colors\n";
    std::cout << "  --plus-force=COLORS - FORCE plus(+) shape (must form cross if enough orbs)\n";
    std::cout << "  --nine-force=COLORS - FORCE 9-grid shape (must form 3x3 if enough orbs)\n";
    std::cout << "  --no-diagonal      - Disable diagonal movement\n";
    std::cout << "  --no-board         - Don't show final board details\n";
    std::cout << "  --no-path          - Don't show route path\n";
    std::cout << "  --no-score         - Don't show scores\n";
    std::cout << "  --no-transform     - Don't show board transformation\n";
    std::cout << "  --verbose          - Show detailed output\n";
    std::cout << "  --help             - Show this help\n\n";
    
    std::cout << "Color Codes:\n";
    std::cout << "  R - Red (Fire)    B - Blue (Water)   G - Green (Wood)\n";
    std::cout << "  L - Light         D - Dark           H - Heal\n\n";
    
    std::cout << "Examples:\n";
    std::cout << "  pazusoba_v1.exe \"RGBLDH...\" --colors=RB --plus=L\n";
    std::cout << "  pazusoba_v1.exe board.txt 3 50 --nine=G --no-diagonal\n";
    std::cout << "  pazusoba_v1.exe \"RGBLDH...\" --plus-force=G --verbose\n";
    std::cout << "  pazusoba_v1.exe \"RGBLDH...\" --nine-force=B --no-board\n\n";
}