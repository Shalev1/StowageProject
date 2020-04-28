#ifndef SHIPPROJECT_SIMTEST_H
#define SHIPPROJECT_SIMTEST_H

#include <iostream>
#include <fstream>

#include <string>
#include "Route.h"
using namespace std;

//---Main class---//
class SimTest {
private:
    string rootDir;
public:
    //---Constructors and Destructors---//
    explicit SimTest(string root);
    SimTest(const SimTest& other) = delete;
    SimTest& operator=(const SimTest& other) = delete;

    void runSimulation();
};

#endif //SHIPPROJECT_SIMTEST_H
