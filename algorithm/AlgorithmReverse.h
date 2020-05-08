#ifndef SHIPPROJECT_ALGORITHMREVERSE_H
#define SHIPPROJECT_ALGORITHMREVERSE_H

#include "BaseAlgorithm.h"

class AlgorithmReverse : public BaseAlgorithm {

protected:
    bool checkMoveContainer(Container *cont, Spot &spot, FileHandler &instructionsFile) override;
};

#endif //SHIPPROJECT_ALGORITHMREVERSE_H
