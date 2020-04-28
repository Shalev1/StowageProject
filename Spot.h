#ifndef STOWAGEPROJECT_SPOT_H
#define STOWAGEPROJECT_SPOT_H

#include <iostream>
#include "Container.h"

using std::pair;

//---Main class---//
class Spot {
private:
    pair<int, int> place;
    Container *cont;
    bool available;
    int floor_num;

public:
    //---Constructors and Destructors---//
    Spot(int x, int y, bool available, int floor_num); //C'tor
    Spot(const Spot &s);                               //Copy C'tor
    Spot &operator=(const Spot &other);

    //---Setters and Getters---//
    pair<int, int> getPlace() const {
        return this->place;
    }

    int getPlaceX() const {
        return this->place.first;
    }

    int getPlaceY() const {
        return this->place.second;
    }

    Container *getContainer() {
        return this->cont;
    }

    bool getAvailable() {
        return this->available;
    }

    int getFloorNum() const {
        return this->floor_num;
    }

    void setPlace(int x, int y, int floor_num) {
        place.first = x;
        place.second = y;
        this->floor_num = floor_num;
    }

    void setContainer(Container *c) {
        this->cont = c;
    }

    void setAvailable(bool avail) {
        this->available = avail;
    }

    //---Class Functions---//

    friend ostream &operator<<(ostream &out, const Spot &s);
};

#endif //STOWAGEPROJECT_SPOT_H
