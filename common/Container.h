#ifndef STOWAGEPROJECT_CONTAINER_H
#define STOWAGEPROJECT_CONTAINER_H

#include <iostream>
#include <cstring>
#include <string>
#include <set>
#include <regex>
#include "ISO_6346.h"

#define NO_WEIGHT (-1)
#define ILLEGAL_WEIGHT (-2)

using std::cout;
using std::string;
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
    bool valid; // will become invalid if ID, weight or dest is illegal or if the ID already exists on port or ship

public:
    //---Constructors and Destructors---//
    Container(int weight, string dest_port, string id, bool valid);
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

    void setPlace(Spot *spot_in_floor) {
        this->spot_in_floor = spot_in_floor;
    }

    bool isValid() const{
        return valid;
    }

    //---Class Functions---//

    friend ostream &operator<<(ostream &out, const Container &c);

    /**
     * Check if the given ID is legal for container
     */
    static bool validateID(const string &id);

};

#endif //STOWAGEPROJECT_CONTAINER_H
