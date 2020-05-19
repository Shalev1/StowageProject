//
// Created by tomer on 17/05/2020.
//

#include "_206223976_c.h"
REGISTER_ALGORITHM(_206223976_c)

void _206223976_c::markRemoveContainers(Container &cont, Spot &spot, vector<Container *> &reload_containers,
                                        FileHandler &instructionsFile) {
    if(spot.getFloorNum() == 0 && spot.getPlaceX() == 0 && spot.getPlaceY() == 0)
        return;
    BaseAlgorithm::markRemoveContainers(cont, spot, reload_containers, instructionsFile);
}