//
// Created by Shalev on 11-Apr-20.
//

#include "WeightBalanceCalculator.h"

WeightBalanceCalculator::WeightBalanceCalculator() {}

bool WeightBalanceCalculator::weightCheck(const ShipPlan &ship, const Container &cont) const {
    (void) ship;
    return (cont.getWeight() >= 0); // Always True
}

bool WeightBalanceCalculator::balanceTest(const ShipPlan &ship, const Container &cont, const Spot &spot) const {
    (void) ship;
    (void) spot;
    return (cont.getWeight() >= 0); // Always True
}