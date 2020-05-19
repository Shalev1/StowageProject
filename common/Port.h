/**
 * Created by Tomer Yoeli
 * Class to represent a port in the travel
 * In charge of init and handle the containers that wait in this port
 */

#ifndef SHIPPROJECT_PORT_H
#define SHIPPROJECT_PORT_H

#include <vector>
#include <string>
#include <map>
#include <cctype>
#include "Container.h"
#include "Utils.h"
#include "algorithm"
#include "ShipPlan.h"

using std::pair;
using std::map;

//---Defines---//
#define PORT_NAME_LEN 5
#define INVALID "Invalid"

//---Main class---//
class Port {
private:
    string name; // 5 letters represents the port code
    vector<Container> waitingContainers; // Containers waiting in this port to be loaded to the ship
    map<string, int> duplicateIdOnPort; // map from id to the number of duplicates on the port
                                        // (value of 1 means total of 2 containers with this id)

public:
    //---Constructors and Destructors---//
    explicit Port(const string &name);

    const string &getName() const{
        return name;
    }

    /**
     * Validate that name is legal port code
     */
    static bool validateName(const string &name);

    vector<Container>& getWaitingContainers() {
        return waitingContainers;
    }

    /**
     * Read the file locate in @param path to initialize the waiting containers vector
     * @param errVector filled with errors that occurs
     */
    void initWaitingContainers(const string &path, vector<pair<int,string>>& errVector, const ShipPlan& ship);

    /**
     * @param skipInvalid: true if the search is among valid containers only
     * Return the container with id equals to @param id or nullptr if there is not one like that
     */
    Container* getWaitingContainerByID(const string &id, bool skipInvalid = true);

    /**
    * Return the container with id equals to @param id or nullptr if there is not one like that
    * @param skipInvalid: true if the search is among valid containers only
    */
    static Container* getContainerByIDFrom(vector<Container>& containers, const string& id, bool skipInvalid = true);

    /**
     * Return a set of IDs waiting at the port.
     */
    set<string> getContainersIDFromPort();

    /**
     * return @param name is uppercase format
     */
    string nameToUppercase(const string &name);

    /**
     * Is cont's ID already appear on the port
     * @param cont should be invalid, in case valid container is given return false (not duplicate)
     */
    bool isDuplicateOnPort(Container& cont);

    int getNumOfDuplicates(const string& id);

    void decreaseDuplicateId(const string& id);

    friend ostream &operator<<(ostream &os, const Port &p);
};

#endif //SHIPPROJECT_PORT_H
