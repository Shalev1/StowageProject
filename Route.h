#ifndef SHIPPROJECT_ROUTE_H
#define SHIPPROJECT_ROUTE_H

#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include "Port.h"
#include "Utils.h"

#define indexOfFirst_InPath (6)

//---Main class---//
class Route {
private:
    int currentPortNum; // current port number in ports, initialize to -1 until start moving
    vector<Port> ports; // The destination in the current route
    string dir; // The directory of the files
    vector<string> portsContainersPaths; // Contain relative paths to the containers files, that have not used yet

    /**
     * Call in the constructor, init the route from the given path file
     */
    void initRouteFromFile(const string &path);

public:
    explicit Route(const string &path);

    /**
     * Sort the given paths for containers files base on the asked sorting formula
     * dir is the base directory and paths are relative path in this directory
     */
    void initPortsContainersFiles(const string &dir, vector<string> &paths);

    /**
     * Return if there is at least one more port in the route
     */
    bool hasNextPort() const {
        return currentPortNum < (int) (ports.size()) - 1;
    }

    /**
     * Return the true if there is at least one more port in the route
     * Also load the waiting containers in this port
     */
    bool moveToNextPort();

    Port &getCurrentPort() {
        return ports[currentPortNum];
    }

    /**
     * Get the closer destination in the route between the two given ones
     */
    string getCloserDestination(const string &d1, const string &d2);

    /**
     * Check if port is in the route (searching from the current port)
     */
    bool isInRoute(const string &portName) const;

    /**
     * Sort the given containers vector by their destination, from the closest one to the farthest one
     */
    void sortContainersByDestination(vector<Container *> &containers);


    /**
     * Delete all containers that stay in the port
     */
    void clearCurrentPort();

    friend ostream &operator<<(ostream &os, const Route &r);
};

#endif //SHIPPROJECT_ROUTE_H
