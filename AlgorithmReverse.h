#ifndef SHIPPROJECT_ALGORITHMREVERSE_H
#define SHIPPROJECT_ALGORITHMREVERSE_H

#include "BaseAlgorithm.h"

class AlgorithmReverse : public BaseAlgorithm {

protected:
    Spot *getEmptySpot(int &returnFloorNum) override;
};

#endif //SHIPPROJECT_ALGORITHMREVERSE_H
