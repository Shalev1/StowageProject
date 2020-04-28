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
    const string id;
    static set<string> ids; // saves all of the ids that currently in use
public:
    //---Constructors and Destructors---//
    Container(int weight, string dest_port, const string id);

    Container &operator=(const Container &c) = delete;

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
     * @param newOne : is this function check validation for
     *      new container or not (need to consider uynique ID or not)
     */
    static bool validateID(const string &id, bool newOne = true);

    /**
     * Clearing all containers that were used in a specific travel.
     */
    static void clearIDs() {
        ids.clear();
    }
};

#endif //STOWAGEPROJECT_CONTAINER_H
