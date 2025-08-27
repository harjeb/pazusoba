/*
main_v2.cpp
The entry point of pazusoba. It handles command line arguments

Created by Yiheng Quan on 12/11/2020
*/

#include <iostream>
#include "core/v2/core.h"

int main(int argc, char *argv[])
{
    using namespace std;
    
    cout << "Pazusoba Solver V2" << endl;
    cout << "Note: V2 implementation is incomplete." << endl;
    cout << "Please use pazusoba_v1.exe for full functionality." << endl;
    
    if (argc > 1) {
        cout << "Arguments provided: ";
        for (int i = 1; i < argc; i++) {
            cout << argv[i] << " ";
        }
        cout << endl;
    }
    
    cout << "V2 solver would process here..." << endl;
    cout << "Program completed." << endl;
    
    return 0;
}
