#ifndef SHIPPROJECT_BASEALGORITHM_H
#define SHIPPROJECT_BASEALGORITHM_H

#include "Route.h"
#include "ShipPlan.h"
#include "WeightBalanceCalculator.h"
#include "AbstractAlgorithm.h"
#include <map>

using std::map;

// Base class of an algorithm, all the base functions for the algorithm have trivial implementations
// and virtual to allow override them with other implementation of different algorithms
class BaseAlgorithm : AbstractAlgorithm{
protected:
    ShipPlan ship;
    Route route;
    WeightBalanceCalculator weightCal;

    /**
     * Unload all the containers that their destination is portName
     * Also unload the containers above them and insert them to reload_containers vector
     */
    virtual void getUnloadInstructions(const string &portName, vector<Container *> &reloadContainers, FileHandler &instructionsFile);

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
     * Check it's possible to use move instruction for @param cont from the ship (places in @param spot)
     * If it's possible, write move instruction in the instructions file
     * Return true if succeed and false if fails
     */
    virtual bool checkMoveContainer(Container* cont, Spot& spot, FileHandler& instructionsFile);

    /**
     * Remove @param cont from the ship (places in @param spot)
     * Also Unload all the containers above it and insert them in reloadContainers
     */
    virtual void markRemoveContainers(Container &cont, Spot &spot, vector<Container *> &reload_containers,
                                      FileHandler &instructionsFile);

public:

    /**
     *  Fill the instructions file with the instructions that need to do in this port.
     *  Get the containers to be loaded in this port from the input file
     */
    virtual int getInstructionsForCargo(const std::string &input_full_path_and_file_name, const std::string &output_full_path_and_file_name) override;

    /**
     * Read the ship plan from the given file
     */
    virtual int readShipPlan(const std::string &full_path_and_file_name) override;

    /**
     * Read the ship route from the given file
     */
    virtual int readShipRoute(const std::string &full_path_and_file_name) override;

    /**
     * Set the weight balance calculator from the given one
     */
    virtual int setWeightBalanceCalculator(WeightBalanceCalculator &calculator) override;

    virtual ~BaseAlgorithm() = default;
};

#endif //SHIPPROJECT_BASEALGORITHM_H