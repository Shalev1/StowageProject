#ifndef SHIPPROJECT_PORT_H
#define SHIPPROJECT_PORT_H

#include <vector>
#include <string>
#include <cctype>
#include "Container.h"
#include "Utils.h"
#include "algorithm"

using std::pair;

//---Defines---//
#define PORT_NAME_LEN 5
#define INVALID "Invalid"

//---Main class---//
class Port {
private:
    string name; // 5 letters represents the port code
    vector<Container> waitingContainers; // Containers waiting in this port to be loaded to the ship

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
     * Add container to the waiting containers vector
     */
    void addContainer(int weight, const string &destPort, const string &id);

    /**
     * Read the file locate in @param path to initialize the waiting containers vector
     * @param errVector filled with errors that occurs
     */
    void initWaitingContainers(const string &path, vector<pair<int,string>>& errVector);

    /**
     * Return the container with id equals to @param id or nullptr if there is not one like that
     */
    Container* getWaitingContainerByID(const string &id);

    /**
    * Return the container with id equals to @param id or nullptr if there is not one like that
    */
    static Container* getContainerByIDFrom(vector<Container>& containers, const string& id);

    /**
     * return @param name is uppercase format
     */
    string nameToUppercase(const string &name);

    friend ostream &operator<<(ostream &os, const Port &p);
};

#endif //SHIPPROJECT_PORT_H
