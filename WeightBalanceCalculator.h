#ifndef STOWAGEPROJECT_WEIGHTBALANCECALCULATOR_H
#define STOWAGEPROJECT_WEIGHTBALANCECALCULATOR_H

#include "ShipPlan.h"
#include "Container.h"


class WeightBalanceCalculator {

public:
    WeightBalanceCalculator();

    /**
     * Check if the given container has legal weight.
     */
    bool weightCheck(const ShipPlan &ship, const Container &cont) const;

    /**
     * Check if the given container at the given spot makes the ship unbalanced.
     */
    bool balanceTest(const ShipPlan &ship, const Container &cont, const Spot &spot) const;

};


#endif //STOWAGEPROJECT_WEIGHTBALANCECALCULATOR_H
