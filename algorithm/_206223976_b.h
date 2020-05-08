#ifndef SHIPPROJECT__206223976_B_H
#define SHIPPROJECT__206223976_B_H

#include "_206223976_a.h"

class _206223976_b : public _206223976_a {

protected:
    bool checkMoveContainer(Container *cont, Spot &spot, FileHandler &instructionsFile) override;
};

#endif //SHIPPROJECT__206223976_B_H
