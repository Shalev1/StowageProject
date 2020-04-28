#ifndef SHIPPROJECT_ALGORITHMREVERSE_H
#define SHIPPROJECT_ALGORITHMREVERSE_H

#include "Algorithm.h"

class AlgorithmReverse : public Algorithm {

public:
    AlgorithmReverse(const ShipPlan &plan, Route *travel, WeightBalanceCalculator *cal) : Algorithm{plan, travel,
                                                                                                    cal} {}

protected:
    Spot *getEmptySpot(int &returnFloorNum) override;
};

#endif //SHIPPROJECT_ALGORITHMREVERSE_H
