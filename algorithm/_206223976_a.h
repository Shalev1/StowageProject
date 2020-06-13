/**
 * Created by Tomer Yoeli
 * class of Algorithm that use all aff the default implementation, without override any of them
 */

#ifndef SHIPPROJECT__206223976_A_H
#define SHIPPROJECT__206223976_A_H

#include "../algorithm/BaseAlgorithm.h"


// Algorithm that use all of the default functions
class _206223976_a : public BaseAlgorithm{

private:
    Spot *getEmptySpot(Container* cont, int fromX = -1, int fromY = -1) override;
};

#endif //SHIPPROJECT__206223976_A_H