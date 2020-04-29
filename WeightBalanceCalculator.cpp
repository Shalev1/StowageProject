//
// Created by Shalev on 11-Apr-20.
//

#include "WeightBalanceCalculator.h"

WeightBalanceCalculator::BalanceStatus WeightBalanceCalculator::tryOperation(char loadUnload, int kg, int X, int Y) {
    (void)(kg+X+Y+loadUnload); // Just to use these variables and prevent warnings
    return APPROVED;
}