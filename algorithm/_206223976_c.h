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
};