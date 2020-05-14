#ifndef STOWAGEPROJECT_FLOOR_H
#define STOWAGEPROJECT_FLOOR_H

#include <set>
#include "Spot.h"


using std::pair;
using std::vector;
using std::set;

//---Main class---//
class Floor {
private:
    int floor_num;
    int rows;
    int cols;
    vector<vector<Spot>> floor_map;

public:
    //---Constructors and Destructors---//
    Floor(int floor_num, int row, int col); // C'tor

    //---Setters and Getters---//
    int getFloorNum() const {
        return this->floor_num;
    }

    int getFloorRows() const {
        return this->rows;
    }

    int getFloorCols() const {
        return this->cols;
    }

    vector<vector<Spot>> &getFloorMap() {
        return this->floor_map;
    }

    void setFloorNum(int floor_num) {
        this->floor_num = floor_num;
    }

    void setFloorCol(int cols) {
        this->cols = cols;
    }

    void setFloorRow(int rows) {
        this->rows = rows;
    }

    //---Class Functions---//

    friend ostream &operator<<(ostream &out, const Floor &f);

    /**
     * Initializes floor's map
     */
    void initializeFloor();

    void initializeFloor(const set<pair<int, int>> &unavailable_spots);

    Container *getContainerAt(int x, int y) {
        return floor_map[x][y].getContainer();
    }

    Spot &getSpotAt(int x, int y) {
        return floor_map[x][y];
    }

    /**
     * Returns a vector of unavailable spots at the floor.
     */
    set<pair<int, int>> getUnavailableSpots() const;
};

#endif //STOWAGEPROJECT_FLOOR_H
