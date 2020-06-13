/**
 * Created by Tomer Yoeli
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
    bool shipValid = true; // Is ship created successfully
    int shipErrorCode = 0; // last fatal error code in ship init, 0 if there wasn't any
    Route route;
    bool routeValid = true; // Is route created successfully
    int routeErrorCode = 0; // last fatal error code in route init, 0 if there wasn't any
    WeightBalanceCalculator weightCal;
    vector<int> errorCodeBits; // values for each error code bit

    /**
     * Unload all the containers that their destination is portName
     * Also unload the containers above them and insert them to reload_containers vector
     */
    virtual void getUnloadInstructions(const string &portName, vector<Container *> &reloadContainers, FileHandler &instructionsFile);

    /**
     * Unload all the containers that there is no container on them with different destination
     */
    virtual void firstUnloading(const string &portName, FileHandler &instructionsFile);

    /**
     * Search for an empty spot in the ship for container loading
     * @param fromX and fromY: find empty spot to container in (fromX, fromY), disable same column spot
     * virtual function, each algorithm implement it different
     */
    virtual Spot *getEmptySpot(Container* cont,  int fromX = -1, int fromY = -1) = 0;

    /**
     * Search for an empty spot in the first floor that available (scan all rows and columns)
     */
    virtual Spot *searchFirstFloor();

    /**
     * Search for an empty spot on container with the same destination
     * @param backup is out param, will be true if the spot belong to uniqueDestSpot
     */
    virtual Spot *searchSameDest(Container* cont, int fromX, int fromY, bool& unique);

    /**
     *  Scan the ship twice, in the first scan search for spot where all of the containers below
     *  have further destination then @param cont's destination.
     *  The second scan will be done if the first one fails, search for an empty spot without any conditions
     */
    virtual Spot *scanShip(Container* cont, int fromX, int fromY);

    /**
     * Load container to the ship, return false if ship is full
     */
    virtual bool findLoadingSpot(Container *cont, FileHandler &instructionsFile);

    /**
     * Check if it's possible to use move instruction for @param cont from the ship (places in @param spot)
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

    /**
     * fill @param loadingContainers with reference to all of the containers that will be
     * loaded to the ship (including the reloaded containers)
     * @param rejectContainers - containers that will be rejected because the ship is full
     */
    virtual void getLoadingContainers(const vector<Container*>& reloadContainers, FileHandler& instructionsFile,
                                      vector<Container*>& loadingContainers, vector<Container*>& rejectContainers);

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
