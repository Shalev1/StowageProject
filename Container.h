#ifndef STOWAGEPROJECT_CONTAINER_H
#define STOWAGEPROJECT_CONTAINER_H

#include <iostream>
#include <cstring>
#include <string>
#include <set>
#include <regex>

using std::cout;
using std::string;
using std::set;
using std::ostream;
using std::endl;
using std::regex;


class Spot;

//---Main class---//
class Container {
private:
    int weight;
    string dest_port;
    Spot *spot_in_floor;
    string id;
    static set<string> simIds; // saves all of the ids that currently in use
    static set<string> algoIds; // saves all of the ids that currently in use
public:
    //---Constructors and Destructors---//
    Container(int weight, string dest_port, string id, bool algoCont = false);
    bool operator== (const Container& c) {
        return id == c.id;
    }

    //---Setters and Getters---//
    int getWeight() const {
        return this->weight;
    }

    string getDestPort() const;

    string getID() {
        return this->id;
    }

    Spot *getSpotInFloor() const {
        return this->spot_in_floor;
    }

    void setWeight(int weight) {
        this->weight = weight;
    }

    void setDestPort(const string dest_port) {
        this->dest_port = dest_port;
    }

    void setPlace(Spot *spot_in_floor) {
        this->spot_in_floor = spot_in_floor;
    }

    //---Class Functions---//

    friend ostream &operator<<(ostream &out, const Container &c);

    /**
     * Check if the given ID is legal for container
     */
    static bool validateID(const string &id);

    /**
     * Check if the given ID isn't in use
     */
    static bool checkUnique(const string& id, bool algoCheck);

    /**
     * Clearing all containers that were used in a specific travel.
     */
    static void clearIDs() {
        simIds.clear();
        algoIds.clear();
    }
};

#endif //STOWAGEPROJECT_CONTAINER_H
