/**
 * class of Algorithm that can use move instructions
 */

#ifndef SHIPPROJECT__206223976_B_H
#define SHIPPROJECT__206223976_B_H

#include "BaseAlgorithm.h"

class _206223976_b : public BaseAlgorithm {

protected:
    bool checkMoveContainer(Container *cont, Spot &spot, FileHandler &instructionsFile) override;
};

#endif //SHIPPROJECT__206223976_B_H
