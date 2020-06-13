#include "_206223976_b.h"
REGISTER_ALGORITHM(_206223976_b)

Spot *_206223976_b::getEmptySpot(Container* cont, int fromX, int fromY) {
    Spot* emptySpot = nullptr;

    if(route.stopsUntilPort(cont->getDestPort()) >= 0.8 * route.stopsLeft()) { // The destination is far away
        emptySpot = searchFirstFloor();
        if(emptySpot != nullptr){
            return emptySpot;
        }
    }

    // Try To load the container on container with same destination (for good spot)
    bool uniqueSpot;
    emptySpot = searchSameDest(cont, fromX, fromY, uniqueSpot);
    if(emptySpot != nullptr){
        return emptySpot;
    }

    // Now scan the ship from bottom to top
    emptySpot = scanShip(cont, fromX, fromY);
    return emptySpot;
}