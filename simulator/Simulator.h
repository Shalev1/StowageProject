#ifndef STOWAGEPROJECT_SIMULATOR_H
#define STOWAGEPROJECT_SIMULATOR_H

#include <string>
#include <iostream>
#include <map>
#include <search.h>
#include <dlfcn.h>
#include <filesystem>
#include "../common/ShipPlan.h"
#include "../common/Route.h"
#include "../interfaces/AbstractAlgorithm.h"
#include "../common/Utils.h"
#include "../interfaces/WeightBalanceCalculator.h"


#define NUM_OF_ALGORITHMS 2

using std::to_string;

//---Main class---//
class Simulator {
private:
    ShipPlan ship;
    Route travel;
    string output_dir_path;
    bool err_in_travel;
    bool err_detected;
    vector<vector<string>> statistics;
    vector<vector<string>> errors;
    string curr_travel_name;
    string curr_port_name;
    static Simulator inst;
    vector<pair<string, std::function<std::unique_ptr<AbstractAlgorithm>()>>> algo_funcs;

    inline static map<string, AbstractAlgorithm::Action> actionDic = {{"L", AbstractAlgorithm::Action::LOAD},
                                                                      {"U", AbstractAlgorithm::Action::UNLOAD},
                                                                      {"M", AbstractAlgorithm::Action::MOVE},
                                                                      {"R", AbstractAlgorithm::Action::REJECT}};

    inline static map<int, string> errCodes = {{0,  "ship plan: a position has an equal number of floors or more than the number of floors provided in the first line (ignored)"},
                                               {1,  "ship plan: a given position exceeds the X/Y ship limits (ignored)"},
                                               {2,  "ship plan: bad line format after first line or duplicate x,y appearance with same data (ignored)"},
                                               {3,  "ship plan: travel error - bad first line or file cannot be read altogether (cannot run this travel)"},
                                               {4,  "ship plan: travel error - duplicate x,y appearance with different data (cannot run this travel)"},
                                               {5,  "travel route: a port appears twice or more consecutively (ignored)"},
                                               {6,  " travel route: bad port symbol format (ignored)"},
                                               {7,  "travel route: travel error - empty file or file cannot be read altogether (cannot run this travel)"},
                                               {8,  "travel route: travel error - file with only a single valid port (cannot run this travel)"},
                                               {9,  "reserved"},
                                               {10, "containers at port: duplicate ID on port (ID rejected)"},
                                               {11, "containers at port: ID already on ship (ID rejected)"},
                                               {12, "containers at port: bad line format; missing or bad weight (ID rejected)"},
                                               {13, "containers at port: bad line format; missing or bad port dest (ID rejected)"},
                                               {14, "containers at port: bad line format; ID cannot be read (ignored)"},
                                               {15, "containers at port: illegal ID check ISO 6346 (ID rejected)"},
                                               {16, "containers at port: file cannot be read altogether (assuming no cargo to be loaded at this port)"},
                                               {17, "containers at port: last port has waiting containers (ignored)"},
                                               {18, "containers at port: total containers amount exceeds ship capacity (rejecting far containers)"}};


    /**
     * Iterates over the given travel folder and initializes the ship plan and the route.
     */
    bool scanTravelDir(int num_of_algo, string &plan_path, string &route_path, vector<string> &travel_files, const std::filesystem::path &travel_dir);

    /**
     * Executing the travel simulation-
     */
    void executeTravel(int num_of_algo, const string& algo_name, AbstractAlgorithm &algo, WeightBalanceCalculator &calc, vector<pair<int, string>> &errs_in_ctor, int &num_of_errors);

    /**
     * Performs the instructions at the given instructions file while validating the algorithm decisions.
     */
    void
    implementInstructions(WeightBalanceCalculator &calc,
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
    bool validateRejectOp(int num_of_algo, ShipPlan &ship, Route &travel, WeightBalanceCalculator &calc,
                          int floor_num, int x, int y, const string &cont_id, bool &has_potential_to_be_loaded);

    /**
     * Checks that the right containers were left at the port when the ship is leaving.
     */
    void
    checkRemainingContainers(map<string, Container *> &unloaded_containers,
                             map<string, Container *> &rejected_containers,
                             Port &curr_port, Route &travel, int num_of_algo, int num_free_spots);

    /**
     * Checks that there was no containers left on the ship destinated for the given port.
     */
    void checkMissedContainers(const string &port_name, int num_of_algo);

    /**
     * Merging given errors with the errors member.
     */
    void extractGeneralErrors(vector<pair<int, string>> &err_strings);

    /**
     * Updating algorithm and output path to be the curret folder if they are missing.
     */
    bool updateInput(string &algorithm_path);

    /**
     * Sort the results, first appears algorithms with lowest number of errors, and in case errors number
     * is even sort by sum of actions that the algorithm done
     */
    void sortResults();

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
     * Receives an integer representing error codes and add them to the errors log accordingly.
     */
    void analyzeErrCode(int err_code, int num_of_algo);

public:
    //---Constructors and Destructors---//
    explicit Simulator(const string &root);

    Simulator() = default;

    static Simulator& getInstance(){
        return inst;
    }
    void registerAlgorithm(std::function<std::unique_ptr<AbstractAlgorithm>()> algo_ctor) {
        algo_funcs.emplace_back("", algo_ctor);
    }

    /**
     * Main function that runs the simulation.
     */
    bool runSimulation(string algorithm_path, string output_path);

    /**
     * Prints the simulation results.
     */
    void printSimulationResults();

    /**
     * Prints the simulation errors.
     */
    void printSimulationErrors();

};

#endif //STOWAGEPROJECT_SIMULATOR_H
