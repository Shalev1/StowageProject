#ifndef SHIPPROJECT_PORT_H
#define SHIPPROJECT_PORT_H

#include <vector>
#include <string>
#include <cctype>
#include "Container.h"
#include "Utils.h"
#include "algorithm"

//---Defines---//
#define PORT_NAME_LEN 6
#define SPACE_LOC_IN_NAME 2
#define INVALID "Invalid"

//---Main class---//
class Port {
private:
    string name;                           // 2 letters country code, space, 3 letters port code
    vector<Container *> waitingContainers; // Containers waiting in this port to be loaded to the ship

public:
    //---Constructors and Destructors---//
    explicit Port(const string &name);

    Port(const Port &other) = delete;

    Port &operator=(const Port &other) = delete;

    const string &getName() {
        return name;
    }

    /**
     * Validate that name is legal port code
     */
    static bool validateName(const string &name);

    vector<Container *> &getWaitingContainers() {
        return waitingContainers;
    }

    /**
     * Add container to the waiting containers vector
     */
    void addContainer(int weight, const string &destPort, const string &id);

    /**
     * Read the file locate in @param path to initialize the waiting containers vector
     */
    void initWaitingContainers(const string &path);

    /**
     * Return the container with id equals to @param id or nullptr if there is not one like that
     */
    Container* getWaitingContainerByID(const string &id);

    /**
    * Return the container with id equals to @param id or nullptr if there is not one like that
    */
    static Container *getContainerByIDFrom(const vector<Container*>& containers, const string &id);

    string nameToUppercase(const string &name);

    friend ostream &operator<<(ostream &os, const Port &p);
};

#endif //SHIPPROJECT_PORT_H
