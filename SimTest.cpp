#include "SimTest.h"
#include "Port.h"
#include "Utils.h"
#include "Algorithm.h"
#include "ShipPlan.h"

#include <utility>

SimTest::SimTest(string root)  : rootDir(move(root)){}

void SimTest::runSimulation() {
    //TODO: loop over all sub directories
    string subDir = rootDir + "\\travel2";
    //TODO: loop inside subDir to get all files (ship plan, route, containers files)
    string routePath = subDir + "\\Route.csv";
    Route* route = new Route(routePath);
    vector<string> portPaths;
    portPaths.emplace_back("AA FDG_1.cargo_da.csv");
    portPaths.emplace_back("AA FDG_1.cargo_data.csv");
    portPaths.emplace_back("AA FDG_20.cargo_data.csv");
    portPaths.emplace_back("AA FDG_rt.cargo_data.csv");
    portPaths.emplace_back("AADDFF_1.cargo_data.csv");
    portPaths.emplace_back("CC FDG_1.cargo_data.csv");
    portPaths.emplace_back("BB FDG_1.cargo_data.csv");
    route->initPortsContainersFiles(subDir, portPaths);
    ShipPlan* ship = new ShipPlan(1,1,1);
    WeightBalanceCalculator cal;
    Algorithm algo(*ship, *route, cal);
//    cout << route << endl;
    route->moveToNextPort();
    cout << route->getCurrentPort() << endl;
//    cout << route->getCurrentPort() << endl;
//    algo.getInstructionsForCargo(route->getCurrentPort().getWaitingContainers());
//    cout << route->getCurrentPort() << endl;

    //cout << route << endl;
}