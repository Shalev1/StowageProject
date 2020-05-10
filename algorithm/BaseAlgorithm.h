/**
 * An abstract class, implement all of the algorithm function with trivial implementations,
 * allow other algorithms to override some off the functions with different algorithms
 */

#ifndef SHIPPROJECT_BASEALGORITHM_H
#define SHIPPROJECT_BASEALGORITHM_H

#include "../common/Route.h"
#include "../common/ShipPlan.h"
#include "../interfaces/WeightBalanceCalculator.h"
#include "../interfaces/AbstractAlgorithm.h"
#include "../interfaces/AlgorithmRegistration.h"
#include <map>

#define NUM_OF_ERROR_CODES 19

using std::map;

class BaseAlgorithm : public AbstractAlgorithm{
protected:
    ShipPlan ship;
    Route route;
    WeightBalanceCalculator weightCal;
    vector<int> errorCodeBits; // values for each error code bit

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
    BaseAlgorithm();
    /**
     *  Fill the instructions file with the instructions that need to do in this port.
     *  Get the containers to be loaded in this port from the input file
     */
    int getInstructionsForCargo(const std::string &input_full_path_and_file_name, const std::string &output_full_path_and_file_name) override;

    /**
     * Read the ship plan from the given file
     */
    int readShipPlan(const std::string &full_path_and_file_name) override;

    /**
     * Read the ship route from the given file
     */
    int readShipRoute(const std::string &full_path_and_file_name) override;

    /**
     * Set the weight balance calculator from the given one
     */
    int setWeightBalanceCalculator(WeightBalanceCalculator &calculator) override;
};

#endif //SHIPPROJECT_BASEALGORITHM_H
