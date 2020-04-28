#include <iostream>
#include <fstream>
#include "Port.h"
#include "Route.h"
#include "Utils.h"
#include "SimTest.h"
#include "Simulator.h"

int main() {
//    runSim();

    SimTest sim("C:\\Users\\tomer\\CLionProjects\\ShipProject\\Files\\sim");
    sim.runSimulation();
//    vector<Port> destination;
//    destination.emplace_back("AA ADF");
//    destination.emplace_back("BB BNJ");
//    addPort(destination, "CC CGY");

//    for(Port p: destination){
//        cout << p << endl;
//    }

//    cout << p.getName() << endl;
//    cout << r.getDestinations().front().getName() << endl;

//    ifstream fs("C:\\Users\\tomer\\CLionProjects\\ShipProject\\templateRoute.csv");
//    string s2;
//    getline(fs, s2);
//    cout << s2 << endl;

//    getline(fs, s2);
//    cout << s2 << endl;

//    string s = fileToString("C:\\Users\\tomer\\CLionProjects\\ShipProject\\f2.txt");

//    vector<string> vec;
//    getTokens(s2, ",", vec);
//    for(const string& st : vec){
//        cout << st << endl;
//    }
//    cout << vec.size() << endl;

    return 0;
}
