/**
 * Created by Tomer Yoeli
 * class of Algorithm, loading system is firstly try to put the furthest containers at the bottom
 * After it try to put the furthest containers at the bottom
 * If failed, scan the ship twice, first time for smart location and second time for empty spot
 */

#ifndef SHIPPROJECT__206223976_B_H
#define SHIPPROJECT__206223976_B_H

#include "BaseAlgorithm.h"

class _206223976_b : public BaseAlgorithm {

private:
    Spot *getEmptySpot(Container* cont, int fromX = -1, int fromY = -1) override;
};

#endif //SHIPPROJECT__206223976_B_H
