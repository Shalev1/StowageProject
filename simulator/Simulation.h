#ifndef SHIPPROJECT_SIMULATION_H
#define SHIPPROJECT_SIMULATION_H

#include <string>
#include <iostream>
#include <map>
#include <search.h>
#include <filesystem>
#include "../common/ShipPlan.h"
#include "../common/Route.h"
#include "../interfaces/AbstractAlgorithm.h"
#include "../common/Utils.h"
#include "../interfaces/WeightBalanceCalculator.h"
#include "Simulator.h"

using std::to_string;

/**
 * Simulation Class.
 *  Author: Shalev Drukman.
 *  The simulation is responsible for executing a travel according to the given algorithm's instruction.
 *  It makes sure that each instruction is legal and if not, it will be reported the simulator class.
 *  In addition, the simulation collects statistics regarding the algorithm decisions and reports it
 *  to the simulator as well.
 */

//---Main class---//
class Simulation {
private:
    ShipPlan ship;
    Route travel;
    WeightBalanceCalculator calc;
    bool err_in_travel;
    string curr_travel_name;
    string curr_port_name;
    string output_dir_path;
    int num_of_algo;
    int num_of_travel;
    pair<string, std::function<std::unique_ptr<AbstractAlgorithm>()>> algo_name_and_ctor;
    string plan_path;
    string route_path;


    inline static map<string, AbstractAlgorithm::Action> actionDic = {{"L", AbstractAlgorithm::Action::LOAD},
                                                                      {"U", AbstractAlgorithm::Action::UNLOAD},
                                                                      {"M", AbstractAlgorithm::Action::MOVE},
                                                                      {"R", AbstractAlgorithm::Action::REJECT}};

    /**
     * Executing the travel simulation, returns false if any error has occurred.
     */
    bool
    executeTravel(const string &algo_name, AbstractAlgorithm &algo, WeightBalanceCalculator &calc, int &num_of_errors);

    /**
     * Iterate over the instructions file and implementing only it's legal instructions.
     */
    void
    iterateInstructions(WeightBalanceCalculator &calc,
                        const string &instruction_file, int &num_of_operations, int num_of_algo);

    /**
     * Performs the instructions at the given instruction while validating the algorithm decisions.
     */
    void implementInstruction(vector<string> &instruction, AbstractAlgorithm::Action command, int &num_of_operations,
                              int num_of_algo, Port &current_port, WeightBalanceCalculator &calc,
                              map<string, Container *> &rejected_containers,
                              map<string, Container *> &unloaded_containers,
                              int floor_num, int x, int y, Container *cont_to_load);

    /**
     * Validates the instruction format.
     */
    bool validateInstruction(const vector<string> &instructions);

    /**
     * Validates a load instruction.
     */
    bool
    validateLoadOp(Port &curr_port, WeightBalanceCalculator &calc, int floor_num, int x,
                   int y, Container *cont);

    /**
     * Validates a unload instruction.
     */
    bool
    validateUnloadOp(WeightBalanceCalculator &calc, int floor_num, int x,
                     int y, const string &cont_id);

    /**
     * Validates a move instruction.
     */
    bool
    validateMoveOp(WeightBalanceCalculator &calc, int source_floor_num,
                   int source_x, int source_y, int dest_floor_num, int dest_x, int dest_y, const string &cont_id);

    /**
     * Validates a reject instruction.
     */
    bool validateRejectOp(Route &travel,
                          int floor_num, int x, int y, const string &cont_id, bool &has_potential_to_be_loaded);

    /**
     * Checks that the right containers were left at the port when the ship is leaving.
     */
    void
    checkRemainingContainers(map<string, Container *> &unloaded_containers,
                             map<string, Container *> &rejected_containers,
                             Port &curr_port);

    /**
     * Checks that there was no containers left on the ship destinated for the given port.
     */
    void checkMissedContainers(const string &port_name);

    /**
     * Check if all the port containers were loaded on the ship or got rejected.
     */
    void checkPortContainers(vector<string> &ignored_containers, Port &curr_port);

    /**
     * Receives an integer representing error codes and add them to the errors log accordingly.
     */
    void analyzeErrCode(int err_code);

    /**
     * Validates the instruction format and initializes parameters for the verification of instruction.
     */
    bool
    validateCargoInstruction(vector<string> &instruction, vector<string> &ignoredContainers, Container **cont_to_load,
                             Port &current_port, AbstractAlgorithm::Action &command,
                             const map<string, Container *> &unloaded_containers);

    /**
     * Add an error according to the invalid container's details.
     */
    void reportInvalidContainer(Container *cont);

public:
    //---Constructors and Destructors---//
    explicit Simulation(ShipPlan &plan, Route &route, WeightBalanceCalculator &wcalc);

    /**
     * Initialize simulation members with the arguments given.
     */
    void initSimulation(int num_of_algo, int num_of_travel, string &travel_name,
                        pair<string, std::function<std::unique_ptr<AbstractAlgorithm>()>> &algo_p,
                        const string &output_path, const string &plan_path, const string &route_path);

    Simulation() = default;

    /**
     * Main function that runs the simulation.
     */
    bool runSimulation();

};


#endif //SHIPPROJECT_SIMULATION_H
