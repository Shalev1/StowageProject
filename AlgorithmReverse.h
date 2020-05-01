#ifndef SHIPPROJECT_ALGORITHMREVERSE_H
#define SHIPPROJECT_ALGORITHMREVERSE_H

#include "BaseAlgorithm.h"

class AlgorithmReverse : public BaseAlgorithm {

public:
    AlgorithmReverse(const ShipPlan &plan, Route *travel, WeightBalanceCalculator *cal) : BaseAlgorithm{plan, travel,
                                                                                                    cal} {}

protected:
    Spot *getEmptySpot(int &returnFloorNum) override;
};

#endif //SHIPPROJECT_ALGORITHMREVERSE_H
