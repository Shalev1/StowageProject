//
// Created by Shalev on 11-Apr-20.
//

#include <string>
#include "../interfaces/WeightBalanceCalculator.h"

WeightBalanceCalculator::BalanceStatus WeightBalanceCalculator::tryOperation(char loadUnload, int kg, int X, int Y) {
    (void)(kg+X+Y+loadUnload); // Just to use these variables and prevent warnings
    return APPROVED;
}

int WeightBalanceCalculator::readShipPlan(const std::string &full_path_and_file_name) {
    (void)full_path_and_file_name;
    return 0;
}