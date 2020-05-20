//
// Created by tomer on 17/05/2020.
//

#ifndef SHIPPROJECT__206223976_C_H
#define SHIPPROJECT__206223976_C_H

#endif //SHIPPROJECT__206223976_C_H

#include "BaseAlgorithm.h"

class _206223976_c : public BaseAlgorithm {

protected:
    void markRemoveContainers(Container &cont, Spot &spot, vector<Container *> &reload_containers, FileHandler &instructionsFile) override;

    int getInstructionsForCargo(const std::string &input_full_path_and_file_name, const std::string &output_full_path_and_file_name) override;
};