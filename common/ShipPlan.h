#ifndef STOWAGEPROJECT_SHIPPLAN_H
#define STOWAGEPROJECT_SHIPPLAN_H

#include "Floor.h"
#include "Utils.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <set>

using std::set;

/**
 * ShipPlan Class.
 * Author: Shalev Drukman.
 * The class is responsible for initializing and reseting the ship plan. It also
 * maintains the ship properties and provides functions to perform operations on the ship.
 */

//---Main class---//
class ShipPlan {
private:
    int num_of_decks;
    int free_spots_num;
    int rows;
    int cols;
    vector<Floor> decks;
    set<string> containers_ids; // Set of all the containe's IDS that on the ship

public:
    //---Constructors and Destructors---//
    ShipPlan() = default;

    //---Setters and Getters---//
    int getNumOfDecks() const {
        return this->num_of_decks;
    }

    int getNumOfFreeSpots() const {
        return this->free_spots_num;
    }

    int getShipRows() const {
        return this->rows;
    }

    int getShipCols() const {
        return this->cols;
    }

    vector<Floor> &getShipDecks() {
        return this->decks;
    }

    void setNumOfDecks(int num_of_decks) {
        this->num_of_decks = num_of_decks;
    }

    void setShipCols(int cols) {
        this->cols = cols;
    }

    void setShipRows(int rows) {
        this->rows = rows;
    }

    //---Class Functions---//

    void initShipPlanFromFile(const string& file_path, vector<pair<int,string>> &err_msg, bool &success);

    friend ostream &operator<<(ostream &out, const ShipPlan &s);

    bool isFull() const {
        return (this->free_spots_num == 0);
    }

    /**
     * Checks if a spot indexes are within the ship limits
     */
    bool spotInRange(int x, int y) const;

    /**
     * Checks if ship's dimensions are legal.
     */
    bool validateShipSize(int x, int y, int z) const;

    /**
     * Set the relevant spots as unavailable accoring to their floor number.
     */
    void updateSpot(int x, int y, int unavailable_floors);

    /**
     * Functions to insert/delete containers to/from the ship.
     */
    void insertContainer(Spot *pos, Container &cont);

    void insertContainer(int floor_num, int x, int y, Container &cont);

    void removeContainer(Spot *pos);

    void removeContainer(int floor_num, int x, int y);

    /**
     * Relocates a container on the ship.
     */
    void moveContainer(int source_floor_num, int source_x, int source_y, int dest_floor_num, int dest_x, int dest_y);

    /**
     * Returns a vector of containers that were destinated for the given port number.
     */
    vector<Container *> getContainersForDest(const string &port_name);

    Container *getContainerAt(int floor_num, int x, int y) {
        return decks[floor_num].getContainerAt(x, y);
    }

    Spot &getSpotAt(int floor_num, int x, int y) {
        return decks[floor_num].getSpotAt(x, y);
    }

    /**
     * Return the number of unavailable floors for the given spot
     */
    int getUnavailableFloorsNum(int x, int y);

    /**
     * Check if container with id = @param id is on the ship
     */
    bool isContOnShip(const string& id) const;

    /**
     * Clearing the ship members
     */
     void resetShipPlan();
};

#endif //STOWAGEPROJECT_SHIPPLAN_H
