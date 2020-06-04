#include "_206223976_b.h"
REGISTER_ALGORITHM(_206223976_b)

bool _206223976_b::checkMoveContainer(Container *cont, Spot &spot, FileHandler &instructionsFile) {
    Spot* emptySpot = getEmptySpot(cont, spot.getPlaceX(), spot.getPlaceY());
    if(emptySpot != nullptr){
        instructionsFile.writeInstruction("M", cont->getID(), spot.getFloorNum(), spot.getPlaceX(),
                spot.getPlaceY(), emptySpot->getFloorNum(), emptySpot->getPlaceX(), emptySpot->getPlaceY());
        ship.moveContainer(spot.getFloorNum(), spot.getPlaceX(),
                           spot.getPlaceY(), emptySpot->getFloorNum(), emptySpot->getPlaceX(), emptySpot->getPlaceY());
        return true;
    }
    return false;
}