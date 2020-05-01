#include "AlgorithmReverse.h"

Spot *AlgorithmReverse::getEmptySpot(int &returnFloorNum) {
    for (int floor_num = 0; floor_num < ship.getNumOfDecks(); ++floor_num) {
        //Iterate over the current floor's floor map
        for (int x = ship.getShipRows() - 1; x >= 0; --x) {
            for (int y = ship.getShipCols() - 1; y >= 0; --y) {
                Spot *curSpot = &(ship.getSpotAt(floor_num, x, y));
                // Check if the spot is clear base
                if (curSpot->getAvailable() && curSpot->getContainer() == nullptr) {
                    returnFloorNum = floor_num;
                    return curSpot; //Found an available and empty spot
                }
            }
        }
    }
    return nullptr;
}