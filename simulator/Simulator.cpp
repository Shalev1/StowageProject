#include "Simulator.h"

Simulator Simulator::inst;
vector<vector<pair<string, int>>> Simulator::statistics;
vector<vector<vector<string>>> Simulator::errors;

Simulator::Simulator(const string &output_path, unsigned int num_threads) : output_dir_path(output_path),
                                                                            number_of_threads(num_threads),
                                                                            err_occurred(false) {
    vector<vector<string>> first_err_row;
    vector<string> temp_err_row;

    errors.push_back(first_err_row);
    errors[0].push_back(temp_err_row);
    errors[0][0].push_back("General");

    // Create empty file, will be used for port without containers
    FileHandler emptyFile(string(".") + std::filesystem::path::preferred_separator + string("empty_file"), true);


}

void Simulator::initializeResAndErrs() {
    vector<pair<string, int>> new_res_row;
    vector<vector<string>> new_err_row;
    vector<string> temp_err_row;

    statistics.push_back(new_res_row);
    statistics[0].emplace_back("RESULTS", 0);
    for (int num_of_algo = 1; num_of_algo <= (int) inst.algo_funcs.size(); ++num_of_algo) {
        statistics.push_back(new_res_row);
        statistics[num_of_algo].emplace_back(inst.algo_funcs[num_of_algo - 1].first, 0); // insert algorithm name
        errors.push_back(new_err_row);
        errors[num_of_algo].push_back(temp_err_row);
        errors[num_of_algo][0].push_back(inst.algo_funcs[num_of_algo - 1].first); // insert algorithm name
        for (int num_of_travel = 1; num_of_travel <= (int) travel_directories.size(); ++num_of_travel) {
            //Init statistics matrix
            if (num_of_algo == 1)
                statistics[0].emplace_back(travel_directories[num_of_travel - 1].filename(),
                                           0); // creating a travel column
            statistics[num_of_algo].emplace_back("0", 0); // Default statistic value
            //Init errors matrix
            errors[num_of_algo].push_back(temp_err_row);
        }
    }
    statistics[0].emplace_back("Num Errors", 0); // creating a Num Errors column
}

bool Simulator::updateInput(string &algorithm_path) {
    // Output_path update
    if (!this->output_dir_path.empty() && !dirExists(this->output_dir_path)) { // Detected a path that doesnt exist
        errors[0][0].push_back("@ ERROR: Output path that was given does not exist.");
        // Creating the directory
        if (!std::filesystem::create_directories(this->output_dir_path)) { // Couldn't create the directory successfully
            this->output_dir_path = std::filesystem::current_path();
            errors[0][0].push_back("@ FATAL ERROR: Output folder has not been created successfully.");
            return false; // Directory did not open correctly.
        }
    }
    if (this->output_dir_path.empty()) {
        this->output_dir_path = std::filesystem::current_path();
    }
    // Algorithm_path update
    if (!algorithm_path.empty() && !dirExists(algorithm_path)) {
        errors[0][0].push_back("@ FATAL ERROR: Algorithm path that was given is invalid.");
        return false;
    }
    if (algorithm_path.empty())
        algorithm_path = std::filesystem::current_path();
    return true;
}

bool Simulator::scanTravelDir(ShipPlan &ship, Route &travel, string &plan_path, string &route_path,
                              const std::filesystem::path &travel_dir) {
    bool success_build = true, route_found = false, plan_found = false;
    vector<pair<int, string>> errs_in_ctor;
    vector<string> travel_files;

    for (const auto &entry : std::filesystem::directory_iterator(travel_dir)) {
        if (endsWith(entry.path().filename(), ".ship_plan")) { // A ship plan file was found
            if (plan_found) {
                errors[0][0].push_back(
                        "@ Travel: " + this->curr_travel_name + " already found a ship plan file.");
                err_occurred = true;
                continue;
            }
            plan_path = entry.path();
            ship.initShipPlanFromFile(plan_path, errs_in_ctor, success_build);
            plan_found = true;
        } else if (endsWith(entry.path().filename(), ".route")) { // A route file was found
            if (route_found) {
                errors[0][0].push_back("@ Travel: " + this->curr_travel_name + " already found a route file.");
                err_occurred = true;
                continue;
            }
            route_path = entry.path();

            travel = Route();
            travel.initRouteFromFile(route_path, errs_in_ctor, success_build);
            route_found = true;
        } else { // The rest of the files, may include some containers details.
            travel_files.push_back(entry.path().filename());
        }
    }
    if (!plan_found) {
        errors[0][0].push_back("@ Travel: " + this->curr_travel_name + " has no Plan file.");
        err_occurred = true;
        return false;
    }
    if (!route_found) {
        errors[0][0].push_back("@ Travel: " + this->curr_travel_name + " has no Route file.");
        err_occurred = true;
        return false;
    }
    // Adding errors that were detected during Ship and Route builders
    extractGeneralErrors(errs_in_ctor);
    if (!success_build) {
        err_occurred = true;
        return false; //One of the files of the travel is invalid, continue to the next travel.
    }
    travel.initPorts(travel_dir, travel_files, errs_in_ctor, ship);
    extractGeneralErrors(errs_in_ctor);
    travel_files.clear();
    return true;
}

bool Simulator::loadTravelsPaths(string &travels_dir_path) {
    bool empty_travel_dir = true; // Will become false once at least one folder found inside travels folder
    for (const auto &travel_dir : std::filesystem::directory_iterator(
            travels_dir_path)) { // Foreach Travel, do the following:
        if (!std::filesystem::is_directory(travel_dir))
            continue;
        travel_directories.push_back(travel_dir.path());
        if (empty_travel_dir) empty_travel_dir = false;
    }
    if (empty_travel_dir) { // No travel dir found
        errors[0][0].push_back("@ FATAL ERROR: the given travels folder has no sub folders.");
        fillSimErrors();
        err_occurred = true;
        return false;
    }
    return true;
}

bool Simulator::validateAlgoLoad(void *handler, string &algo_name, int prev_size) {
    if (!handler) {
        errors[0][0].push_back("@ ERROR: Dynamic load of algorithm: " + algo_name + " failed:" + dlerror());
        err_occurred = true;
        return false;
    }
    if (prev_size + 1 != (int) inst.algo_funcs.size()) {
        errors[0][0].push_back("@ ERROR: Algorithm: " + algo_name + " did not register successfully.");
        err_occurred = true;
        dlclose(handler);
        return false; //The algorithm did not register successfully.
    }
    return true;
}

void Simulator::loadAlgorithms(string &algorithm_path) {
    int num_of_algo = 0;
    vector<string> algorithm_names = getSOFilesNames(algorithm_path);
    for (auto &algo_name_so : algorithm_names) {
        string algo_name = algo_name_so.substr(0, (int) algo_name_so.length() - 3);
        int prev_size = (int) inst.algo_funcs.size();
        void *handler = dlopen((algorithm_path + std::filesystem::path::preferred_separator + algo_name_so).c_str(),
                               RTLD_LAZY);
        if (!validateAlgoLoad(handler, algo_name, prev_size)) {
            continue; // Algorithm loading failed, continue to the next algorithm.
        }
        handlers.push_back(handler);
        // Set the new algorithm name
        inst.algo_funcs[num_of_algo].first = algo_name;
        num_of_algo++;
    }
}

void Simulator::markRemovedTravel(int num_of_travel) {
    // Editing errors matrix
    for (int num_of_algo = 0; num_of_algo < (int) statistics.size(); ++num_of_algo) {
        statistics[num_of_algo][num_of_travel].second = -1; // Will be ignored when creating results file. Note that the '-1' is not related to the num_of_operations!
    }
}

bool Simulator::checkErrorsDuringSimulations() {
    if(err_occurred) return true;
    for (int i = 1; i < (int) errors.size(); i++) {
        for (int j = 1; j < (int) errors[i].size(); j++) {
            if (!errors[i][j].empty()) {
                return true;
            }
        }
    }
    return false;
}

bool Simulator::start(string algorithm_path, string travels_dir_path) {
    if (!updateInput(algorithm_path)) {
        fillSimErrors();
        err_occurred = true;
        return false;
    }
    if (!dirExists(travels_dir_path)) {
        errors[0][0].push_back("@ FATAL ERROR: Can't find travel directory path.");
        fillSimErrors();
        err_occurred = true;
        return false;
    }

    // Getting ready for the simulation
    if (!loadTravelsPaths(travels_dir_path))
        return false;

    loadAlgorithms(algorithm_path);
    initializeResAndErrs();
    WeightBalanceCalculator calc;

    // Launch simulation!
    ThreadPool thread_pool((int) number_of_threads);
    thread_pool.start(); // In case number of threads is 1, the thread pool is empty and this function does nothing
    for (int num_of_travel = 1; num_of_travel <= (int) travel_directories.size(); ++num_of_travel) {
        string plan_path, route_path;
        this->curr_travel_name = travel_directories[num_of_travel - 1].filename();
        ShipPlan ship;
        Route route;
        //Iterate over the directory and initializing the travel (once!)
        if (!scanTravelDir(ship, route, plan_path, route_path, travel_directories[num_of_travel - 1])) {
            markRemovedTravel(num_of_travel);
            continue; // Fatal error detected. Skip to the next travel.
        }
        for (int num_of_algo = 1; num_of_algo <= (int) inst.algo_funcs.size(); ++num_of_algo) {
            Simulation sim(ship, route, calc);
            sim.initSimulation(num_of_algo, num_of_travel, curr_travel_name, inst.algo_funcs[num_of_algo - 1],
                               output_dir_path, plan_path, route_path);
            if(number_of_threads > 1) // Launch a thread.
                thread_pool.getTask(sim);
            else // This thread is the only one, it is also the worker
                sim.runSimulation();
        }
    }
    thread_pool.finish();

    inst.algo_funcs.clear();
    for (auto &hndl:handlers) { dlclose(hndl); }
    err_occurred = checkErrorsDuringSimulations();
    if (err_occurred) // Errors found, errors_file should be created
        fillSimErrors();
    createResultsFile();

    return true; // No fatal errors were detected
}

void Simulator::extractGeneralErrors(vector<pair<int, string>> &err_strings) {
    if (!err_strings.empty())
        err_occurred = true; //at least one error was found
    for (int i = 0; i < (int) err_strings.size(); ++i) {
        errors[0][0].push_back("@ Travel: " + curr_travel_name + "- " + err_strings[i].second);
    }
    err_strings.clear(); // Clearing the errors list for future re-use.
}

void Simulator::addSumColumn() {
    int sum;
    auto num_of_algos = (int) statistics.size();
    auto num_of_travels = (int) statistics[0].size() - 1; // -1 so we ignore the Errors column.
    statistics[0].insert(statistics[0].end() - 1, pair("Sum", 0));
    for (int i = 1; i < num_of_algos; ++i) {
        sum = 0;
        for (int j = 1; j < num_of_travels; ++j) {
            if (statistics[i][j].first == "-1") continue; // Ignore travels with errors
            sum += string2int(statistics[i][j].first);
        }
        statistics[i].insert(statistics[i].end() - 1, pair(to_string(sum), 0));
    }
}

void Simulator::addNumErrColumn() {
    auto num_of_algos = (int) statistics.size();
    auto num_of_travels = (int) statistics[0].size() - 1;
    int num_err;
    for (int i = 1; i < num_of_algos; ++i) {
        num_err = 0;
        for (int j = 1; j < num_of_travels; ++j) {
            if (statistics[i][j].second == -1) continue; // Ignore invalid travels
            num_err += statistics[i][j].second;
        }
        statistics[i].emplace_back(to_string(num_err), 0);
    }
}

void Simulator::sortResults() {
    sort(statistics.begin(), statistics.end(), [](vector<pair<string, int>> &v1, vector<pair<string, int>> &v2) {
        if (v1[0].first == "RESULTS") // Title row, always first
            return true;
        if (v2[0].first == "RESULTS") // Title row, always first
            return false;
        int v1_errors = stoi(v1[(int) v1.size() - 1].first);
        int v2_errors = stoi(v2[(int) v2.size() - 1].first);
        if (v1_errors != v2_errors) // Sort by number of errors, if there is difference
            return v1_errors < v2_errors;
        // Sort by sum of actions in case of errors tie
        return stoi(v1[(int) v1.size() - 2].first) < stoi(v2[(int) v2.size() - 2].first);
    });
}

void Simulator::createResultsFile() {
    FileHandler res_file(this->output_dir_path + std::filesystem::path::preferred_separator + "simulation.results",
                         true);
    if (res_file.isFailed()) {
        return;
    }
    addNumErrColumn();
    addSumColumn();
    sortResults();
    auto rows = (int) statistics.size();
    auto cols = (int) statistics[0].size();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (j == cols - 1) { // in case we are in the last cell
                // Skip the travel data if it got skipped
                (statistics[i][j].second != -1) ? res_file.writeCell(statistics[i][j].first, true) : res_file.writeCell(
                        "", true);
            } else if (statistics[i][j].second != -1) res_file.writeCell(statistics[i][j].first);
        }
    }
}

bool noErrorsDetected(vector<vector<string>> &errors) {
    for (int i = 1; i < (int) errors.size(); ++i) { // Skip the first cell since it's the name of algorithm cell.
        if (!errors[i].empty())
            return false;
    }
    return true;
}

void printGeneralErrors(FileHandler &err_file, vector<string> &err_msgs) {
    err_file.writeCell("________________" + err_msgs[0] + " Errors________________", true);
    for (int i = 1; i < (int) err_msgs.size(); ++i) {
        err_file.writeCell(err_msgs[i], true);
    }

}

void Simulator::fillSimErrors() {
    FileHandler err_file(this->output_dir_path + std::filesystem::path::preferred_separator + "simulation.errors",
                         true);
    if (err_file.isFailed()) {
        return;
    }
    if (errors[0][0].size() > 1) // General Errors section
        printGeneralErrors(err_file, errors[0][0]);

    for (int num_of_algo = 1; num_of_algo < (int) errors.size(); ++num_of_algo) {// Algorithm Errors section
        if (noErrorsDetected(errors[num_of_algo])) {
            // No errors found for this algorithm, skip to the next one
            continue;
        }
        err_file.writeCell("________________" + errors[num_of_algo][0][0] + " Errors________________", true);
        for (int num_of_travel = 1; num_of_travel < (int) errors[num_of_algo].size(); ++num_of_travel) {
            for (int num_of_err = 0; num_of_err < (int) errors[num_of_algo][num_of_travel].size(); ++num_of_err) {
                err_file.writeCell(errors[num_of_algo][num_of_travel][num_of_err], true);
            }
            err_file.writeCell("", true);
        }
    }
}

void Simulator::printSimulationResults() {
    cout << "Results File:" << endl;
    printCSVFile(this->output_dir_path + std::filesystem::path::preferred_separator + "simulation.results");
}

void Simulator::printSimulationErrors() {
    if (!err_occurred) return; // No errors were found
    cout << "Errors File:" << endl;
    if (!printCSVFile(this->output_dir_path + std::filesystem::path::preferred_separator + "simulation.errors"))
        cout << "Couldn't open errors file." << endl;
}

void Simulator::insertResult(int num_of_algo, int num_of_travel, string num_of_op, bool err_in_travel) {
    if (err_in_travel) {
        statistics[num_of_algo][num_of_travel] = pair("-1", 1);
    } else {
        //cout << "Num_of_algo: "<< num_of_algo << " Num of travel: " << num_of_travel << " num of op" << num_of_op << endl;
        statistics[num_of_algo][num_of_travel].first = num_of_op;
    }
}