/**
 * Created by Tomer Yoeli
 * class of Algorithm that use all of the default function except it can use move instructions
 */

#ifndef SHIPPROJECT__206223976_B_H
#define SHIPPROJECT__206223976_B_H

#include "BaseAlgorithm.h"

class _206223976_b : public BaseAlgorithm {

private:
    Spot *getEmptySpot(Container* cont, int fromX = -1, int fromY = -1) override;
};

#endif //SHIPPROJECT__206223976_B_H
