#include "AlgorithmReverse.h"

bool AlgorithmReverse::checkMoveContainer(Container *cont, Spot &spot, FileHandler &instructionsFile) {
    int floorNum;
    Spot* emptySpot = getEmptySpot(floorNum);
    if(emptySpot != nullptr){
        instructionsFile.writeInstruction("M", cont->getID(), spot.getFloorNum(), spot.getPlaceX(),
                spot.getPlaceY(), floorNum, emptySpot->getPlaceX(), emptySpot->getPlaceY());
        ship.moveContainer(spot.getFloorNum(), spot.getPlaceX(),
                           spot.getPlaceY(), floorNum, emptySpot->getPlaceX(), emptySpot->getPlaceY());
        return true;
    }
    return false;
}