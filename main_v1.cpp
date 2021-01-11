#include <iostream>
// #include "core/v1/solver.h"
#include "core/v1/solver_sat.h"

/// This handles user input from the console, mainly board file and min erase condition
PSolverSAT *handleInput(int, char **);

int main(int argc, char *argv[])
{
    auto soba = handleInput(argc, argv);
    soba->solve();
    delete soba;

    return 0;
}

PSolverSAT *handleInput(int argc, char *argv[])
{
    std::string filePath = "assets/sample_board_floodfill_bug.txt";
    // std::string filePath = "RHGHDRGLBLHGDBLLHBBBHRLHGHDGLB";
    // std::string filePath = "LHHLHDLDDLHLDDLDHDDDLLHDLHDHHDHHLDLDLHDHDH";
    int minErase = 4;
    int maxStep = 50;
    int maxSize = 5000;

    // Read from command line
    if (argc > 1)
    {
        filePath = argv[1];
    }
    if (argc > 2)
    {
        minErase = atoi(argv[2]);
        // min 3, max 5 for now
        if (minErase < 3)
            minErase = 3;
        if (minErase > 5)
            minErase = 5;
    }
    if (argc > 3)
    {
        maxStep = atoi(argv[3]);
    }
    if (argc > 4)
    {
        maxSize = atoi(argv[4]);
    }

    return new PSolverSAT(filePath, minErase, maxStep, maxSize);
}
