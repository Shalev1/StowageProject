#ifndef SHIPPROJECT_BASEALGORITHM_H
#define SHIPPROJECT_BASEALGORITHM_H

#include "Route.h"
#include "ShipPlan.h"
#include "WeightBalanceCalculator.h"
#include "AbstractAlgorithm.h"
#include <map>

using std::map;

enum iType {
    L, U, M, R
};

// Base class of an algorithm, all the base function for the algorithm have trivial implementations
// and virtual to allow override them with other implementation of different algorithms
class BaseAlgorithm {
protected:
    ShipPlan ship;
    Route *route;
    WeightBalanceCalculator *weightCal;

    /**
     * Unload all the containers that their destination is portName
     * Also unload the containers above them and insert them to reload_containers vector
     */
    virtual void
    getUnloadInstructions(const string &portName, vector<Container *> &reloadContainers, FileHandler &instructionsFile);

    /**
     * Reload all the containers that was unload to allow access to lower containers
     */
    virtual void getReloadInstructions(vector<Container *> &reload_containers, FileHandler &instructionsFile);

    /**
     * Search for an empty spot in the ship for container loading
     * @param returnFloorNum is the floor of the founded spot
     */
    virtual Spot *getEmptySpot(int &returnFloorNum);

    /**
     * Load container to the ship
     */
    virtual void findLoadingSpot(Container *cont, FileHandler &instructionsFile);

    /**
     * Remove @param cont from the ship (places in @param spot)
     * Also Unload all the containers above it and insert them in reloadContainers
     */
    virtual void markRemoveContainers(Container &cont, Spot &spot, vector<Container *> &reload_containers,
                                      FileHandler &instructionsFile);

public:
    inline static map<string, iType> dic = {{"L", L},
                                            {"U", U},
                                            {"M", M},
                                            {"R", R}};

    //---Constructors and Destructors---//
    BaseAlgorithm(const ShipPlan &plan, Route *travel, WeightBalanceCalculator *cal) : ship(plan), route(travel),
                                                                                       weightCal(cal) {}

    /**
     *  Fill the instructions vector with the instructions that need to do in this port.
     *  Get the containers to be loaded in this port
     */
    virtual void getInstructionsForCargo(vector<Container *> &loadContainers, const string &instructionsFile);

    virtual ~BaseAlgorithm() = default;
};

#endif //SHIPPROJECT_BASEALGORITHM_H