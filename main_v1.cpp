#include <iostream>
#include <string>
#include <cstring>
#include "core/v1/solver.h"
#include "core/v1/solver_config.h"

/// This handles user input from the console with extended configuration
SolverConfig parseArguments(int argc, char **argv);

int main(int argc, char *argv[])
{
    std::cout << "Pazusoba Solver V1 (Extended) starting..." << std::endl;
    
    // Parse command line arguments
    SolverConfig config = parseArguments(argc, argv);
    
    if (config.verbose) {
        std::cout << "Configuration:" << std::endl;
        std::cout << "  Board: " << config.filePath << std::endl;
        std::cout << "  Min erase: " << config.minErase << std::endl;
        std::cout << "  Max steps: " << config.maxStep << std::endl;
        std::cout << "  Search size: " << config.maxSize << std::endl;
        std::cout << "  Diagonal movement: " << (config.enableDiagonalMovement ? "enabled" : "disabled") << std::endl;
        
        if (!config.priorityColors.empty()) {
            std::cout << "  Priority colors: ";
            for (auto color : config.priorityColors) {
                std::cout << SolverConfig::colorToString(color) << " ";
            }
            std::cout << std::endl;
        }
        
        if (config.enablePlusProfile && !config.plusColors.empty()) {
            std::cout << "  Plus priority colors: ";
            for (auto color : config.plusColors) {
                std::cout << SolverConfig::colorToString(color) << " ";
            }
            std::cout << std::endl;
        }
        
        if (config.enableNineProfile && !config.nineColors.empty()) {
            std::cout << "  Nine-grid priority colors: ";
            for (auto color : config.nineColors) {
                std::cout << SolverConfig::colorToString(color) << " ";
            }
            std::cout << std::endl;
        }
        
        if (config.enablePlusConstraint && !config.plusColors.empty()) {
            std::cout << "  Plus FORCED mode colors: ";
            for (auto color : config.plusColors) {
                std::cout << SolverConfig::colorToString(color) << " ";
            }
            std::cout << std::endl;
        }
        
        if (config.enableNineConstraint && !config.nineColors.empty()) {
            std::cout << "  Nine-grid FORCED mode colors: ";
            for (auto color : config.nineColors) {
                std::cout << SolverConfig::colorToString(color) << " ";
            }
            std::cout << std::endl;
        }
    }
    
    // Set diagonal movement in PState
    PState::setDiagonalMovementEnabled(config.enableDiagonalMovement);
    
    auto soba = new PSolver(config);
    
    auto routes = soba->solve(config);
    
    delete soba;

    return 0;
}

SolverConfig parseArguments(int argc, char **argv)
{
    SolverConfig config;
    
    // Parse positional arguments first
    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--help" || arg1 == "-h") {
            SolverConfig::printUsage();
            exit(0);
        }
        config.filePath = arg1;
    }
    
    if (argc > 2) {
        config.minErase = atoi(argv[2]);
        if (config.minErase < 3) config.minErase = 3;
        if (config.minErase > 5) config.minErase = 5;
    }
    
    if (argc > 3) {
        config.maxStep = atoi(argv[3]);
    }
    
    if (argc > 4) {
        config.maxSize = atoi(argv[4]);
    }
    
    // Parse extended options
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg.find("--colors=") == 0) {
            std::string colors = arg.substr(9);
            config.priorityColors = SolverConfig::parseColorList(colors);
        }
        else if (arg.find("--plus=") == 0) {
            std::string colors = arg.substr(7);
            config.enablePlusProfile = true;
            config.plusColors = SolverConfig::parseColorList(colors);
        }
        else if (arg.find("--nine=") == 0) {
            std::string colors = arg.substr(7);
            config.enableNineProfile = true;
            config.nineColors = SolverConfig::parseColorList(colors);
        }
        else if (arg.find("--plus-force=") == 0) {
            std::string colors = arg.substr(13);
            config.enablePlusConstraint = true;
            config.plusColors = SolverConfig::parseColorList(colors);
        }
        else if (arg.find("--nine-force=") == 0) {
            std::string colors = arg.substr(13);
            config.enableNineConstraint = true;
            config.nineColors = SolverConfig::parseColorList(colors);
        }
        else if (arg == "--no-diagonal") {
            config.enableDiagonalMovement = false;
        }
        else if (arg == "--no-board") {
            config.showFinalBoard = false;
        }
        else if (arg == "--no-path") {
            config.showRoutePath = false;
        }
        else if (arg == "--no-score") {
            config.showScore = false;
        }
        else if (arg == "--no-transform") {
            config.showBoardTransform = false;
        }
        else if (arg == "--verbose") {
            config.verbose = true;
        }
    }
    
    return config;
}
