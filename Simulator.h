#ifndef STOWAGEPROJECT_SIMULATOR_H
#define STOWAGEPROJECT_SIMULATOR_H

#include <string>
#include <iostream>
#include <filesystem>
#include <map>
#include "ShipPlan.h"
#include "Route.h"
#include "BaseAlgorithm.h"
#include "AlgorithmReverse.h"
#include "Utils.h"


#define NUM_OF_ALGORITHMS 2

using std::to_string;

//---Main class---//
class Simulator {
private:
    string rootDir;
    bool err_in_travel;
    bool err_detected;
    vector<vector<string>> statistics;
    vector<vector<string>> errors;

public:
    //---Constructors and Destructors---//
    explicit Simulator(const string &root);

    Simulator(const Simulator &other) = delete;

    Simulator &operator=(const Simulator &other) = delete;

    /**
     * Main function that runs the simulation.
     */
    void runSimulation();

    /**
     * Performs the instructions at the given instructions file while validating the algorithm decisions.
     */
    void
    implementInstructions(ShipPlan &ship, Route *travel, WeightBalanceCalculator &calc,
                          const string &instruction_file, int &num_of_operations, int num_of_algo);

    /**
     * Validates the instruction format.
     */
    bool validateInstruction(const vector<string> &instructions);

    /**
     * Validates a load instruction.
     */
    bool
    validateLoadOp(int num_of_algo, ShipPlan &ship, WeightBalanceCalculator &calc, int floor_num, int x,
                   int y, const Container *cont);

    /**
     * Validates a unload instruction.
     */
    bool
    validateUnloadOp(int num_of_algo, ShipPlan &ship, WeightBalanceCalculator &calc, int floor_num, int x,
                     int y, const string &cont_id);

    /**
     * Validates a move instruction.
     */
    bool
    validateMoveOp(int num_of_algo, ShipPlan &ship, WeightBalanceCalculator &calc, int source_floor_num,
                   int source_x, int source_y, int dest_floor_num, int dest_x, int dest_y, const string &cont_id);

    /**
     * Validates a reject instruction.
     */
    bool validateRejectOp(int num_of_algo, ShipPlan &ship, Route *travel, WeightBalanceCalculator &calc,
                                int floor_num, int x, int y, const string &cont_id, bool &has_potential_to_be_loaded);

    /**
     * Checks that the right containers were left at the port when the ship is leaving.
     */
    void
    checkRemainingContainers(map<string, Container *> unloaded_containers, map<string, Container *> rejected_containers,
                             Port &curr_port, Route *travel, int num_of_algo, int num_free_spots);

    /**
     * Checks that there was no containers left on the ship destinated for the given port.
     */
    void checkMissedContainers(ShipPlan *ship, const string &port_name);

    /**
     * Creating a results file containing the number of operations performed in each travel for each algorithm.
     */
    void createResultsFile();

    /**
     * Calculating the sum of the operations and write it to the result file.
     */
    void addSumColumn();

    /**
     * Creating an errors file containing the errors description that occured during the simulation.
     */
    void fillSimErrors();

    /**
     * Prints the simulation details.
     */
    void printSimulationDetails();
};

#endif //STOWAGEPROJECT_SIMULATOR_H
