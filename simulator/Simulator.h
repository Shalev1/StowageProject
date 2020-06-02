#ifndef STOWAGEPROJECT_SIMULATOR_H
#define STOWAGEPROJECT_SIMULATOR_H

#include <dlfcn.h>
#include "ThreadPool.h"
#include "Simulation.h"


using std::to_string;

/**
 * Simulator Class.
 *  Author: Shalev Drukman.
 *  The simulator is responsible for executing the travels and the algorithms matched together,
 *  using a multi-threading model that creates a thread for each travel-algorithm pair.
 *  Each thread is running a simulation instance that will report errors and statistics to the simulator.
 *  As a result, the simulator is responsible for reporting it to an organized file.
 */

//---Main class---//
class Simulator {
private:
    string output_dir_path;
    unsigned int number_of_threads;
    bool err_occurred;
    string curr_travel_name;

    static vector<vector<pair<string, int>>> statistics;
    // Each cell in the 2D matrix saves a pair of (num_of_operation, error_detected) for an Algorithm-Travel pair.

    static vector<vector<vector<string>>> errors;
    // Each cell in the 2D matrix saves a list of error messages for an Algorithm-Travel pair.

    static Simulator inst;
    vector<pair<string, std::function<std::unique_ptr<AbstractAlgorithm>()>>> algo_funcs;
    vector<void *> handlers;
    vector<std::filesystem::path> travel_directories;

    /**
     * Get all of the travel directories paths.
     */
    bool loadTravelsPaths(string &travels_dir_path);

    /**
     * Loads all the algorithms constructors into the simulator instance list.
     */
    void loadAlgorithms(string &algorithm_path);

    /**
     * Iterates over the given travel folder and initializes the ship plan and the route.
     */
    bool scanTravelDir(ShipPlan &ship, Route &travel, string &plan_path, string &route_path,
                       const std::filesystem::path &travel_dir);

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
     * calculating the number of errors performed by an algorithm and write it to the result file.
     */
    void addNumErrColumn();

    /**
     * Creating an errors file containing the errors description that occured during the simulation.
     */
    void fillSimErrors();

    /**
     * Loads the algorithm constructor function dynamically.
     */
    bool validateAlgoLoad(void *handler, string &algo_name, int prev_size);

    /**
     * Initialize the statistics matrix and the errors matrix (for Travel-Algorithm pair).
     */
    void initializeResAndErrs();

    /**
     * Merging given errors with the errors member.
     */
    void extractGeneralErrors(vector<pair<int, string>> &err_strings);

    /**
     * Mark an invalid travel in the results matrix (will be ignored later).
     */
    void markRemovedTravel(int num_of_travel);

    /**
     * Detects if any error occurred during the simulation.
     */
    void checkErrorsDuringSimulations();

public:
    //---Constructors and Destructors---//
    explicit Simulator(const string &output_path, unsigned int number_of_threads);

    Simulator() = default;

    static Simulator &getInstance() {
        return inst;
    }

    void registerAlgorithm(std::function<std::unique_ptr<AbstractAlgorithm>()> algo_ctor) {
        algo_funcs.emplace_back("", algo_ctor);
    }

    /**
     * Main function that runs the simulation.
     */
    bool start(string algorithm_path, string output_path);

    static void insertError(int num_of_algo, int num_of_travel, string err_msg) {
        errors[num_of_algo][num_of_travel].push_back(err_msg);
    }

    static void insertResult(int num_of_algo, int num_of_travel, string num_of_op, bool err_in_travel);

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
