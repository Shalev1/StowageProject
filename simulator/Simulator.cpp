#include "Simulator.h"

Simulator Simulator::inst;

Simulator::Simulator(const string &root) : output_dir_path(root), err_in_travel(false), err_detected(false),
                                           curr_travel_name("") {
    vector<string> first_res_row;
    vector<string> first_err_row;

    statistics.push_back(first_res_row);
    statistics[0].push_back("RESULTS");
    errors.push_back(first_err_row);
    errors[0].push_back("General");
}

bool Simulator::updateInput(string &algorithm_path) {
    if (!algorithm_path.empty() && !dirExists(algorithm_path)) {
        errors[0].push_back("@ FATAL ERROR: Algorithm path that was given is invalid.");
        return false;
    }
    if (!this->output_dir_path.empty() && !dirExists(this->output_dir_path)) {
        errors[0].push_back("@ FATAL ERROR: Output path that was given is invalid.");
        this->output_dir_path = std::filesystem::current_path();
        return false;
    }

    if (this->output_dir_path.empty()) {
        this->output_dir_path = std::filesystem::current_path();
    }
    if (algorithm_path.empty())
        algorithm_path = std::filesystem::current_path();
    return true;
}

bool Simulator::scanTravelDir(int num_of_algo, string &plan_path, string &route_path, vector<string> &travel_files,
                              const std::filesystem::path &travel_dir) {
    bool success_build = true, route_found = false, plan_found = false;
    vector<pair<int, string>> errs_in_ctor;

    for (const auto &entry : std::filesystem::directory_iterator(travel_dir)) {
        if (endsWith(entry.path().filename(), ".ship_plan")) { // A ship plan file was found
            if (plan_found) {
                if (num_of_algo == 1)
                    errors[0].push_back(
                            "@ Travel: " + this->curr_travel_name + " already found a ship plan file.");
                err_detected = true;
                continue;
            }
            plan_path = entry.path();
            ship = ShipPlan(); // Reset the ship before initialization
            ship.initShipPlanFromFile(plan_path, errs_in_ctor, success_build);
            plan_found = true;
        } else if (endsWith(entry.path().filename(), ".route")) { // A route file was found
            if (route_found) {
                if (num_of_algo == 1)
                    errors[0].push_back("@ Travel: " + this->curr_travel_name + " already found a route file.");
                err_detected = true;
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
        if (num_of_algo == 1) errors[0].push_back("@ Travel: " + this->curr_travel_name + " has no Plan file.");
        err_detected = true;
        return false;
    }
    if (!route_found) {
        if (num_of_algo == 1)
            errors[0].push_back("@ Travel: " + this->curr_travel_name + " has no Route file.");
        err_detected = true;
        return false;
    }
    // Adding errors that were detected during Ship and Route builders
    if (num_of_algo == 1) extractGeneralErrors(errs_in_ctor);
    if (!success_build) {
        err_detected = true;
        return false; //One of the files of the travel is invalid, continue to the next travel.
    }
    return true;
}

void Simulator::executeTravel(int num_of_algo, const string &algo_name, AbstractAlgorithm &algo,
                              WeightBalanceCalculator &calc,
                              vector<pair<int, string>> &errs_in_ctor, int &num_of_errors) {
    string instruction_file_path;
    string instruction_file;
    int num_of_operations = 0;
    //Creating instructions directory for the algorithm
    instruction_file_path = createInstructionDir(output_dir_path, algo_name, curr_travel_name);
    if (instruction_file_path.empty()) {
        cout
                << "ERROR: Failed creating instruction files directory; creates everything inside the output folder."
                << endl;
        instruction_file_path = output_dir_path;
    }
    while (travel.moveToNextPort(errs_in_ctor)) { // For each port in travel
        curr_port_name = travel.getCurrentPort().getName();
        instruction_file =
                instruction_file_path + std::filesystem::path::preferred_separator + curr_port_name + "_" +
                to_string(travel.getNumOfVisitsInPort(curr_port_name)) + ".crane_instructions";
        analyzeErrCode(algo.getInstructionsForCargo(travel.getCurrentPortPath(), instruction_file),
                       num_of_algo);
        (void) algo;
        this->implementInstructions(calc, instruction_file, num_of_operations, num_of_algo);
        this->checkMissedContainers(travel.getCurrentPort().getName(), num_of_algo);
        travel.leaveCurrentPort();
    }
    Container::clearIDs();

    // Check if there was an error by the algorithm. if there was, number of operation is '-1'.
    if (this->err_in_travel) {
        err_detected = true;
        num_of_errors++;
        statistics[num_of_algo].push_back("-1");
    } else {
        statistics[num_of_algo].push_back(to_string(num_of_operations));
    }
}

bool Simulator::validateAlgoLoad(void *handler, string &algo_name, int prev_size) {
    if (!handler) {
        errors[0].push_back("@ FATAL ERROR: Dynamic load of algorithm: " + algo_name + " failed:" + dlerror());
        err_detected = true;
        return false;
    }
    if (prev_size + 1 != (int) inst.algo_funcs.size()) {
        errors[0].push_back("@ FATAL ERROR: Algorithm: " + algo_name + " did not register successfully.");
        err_detected = true;
        return false; //The algorithm did not register successfully.
    }
    return true;
}

bool Simulator::runSimulation(string algorithm_path, string travels_dir_path) {
    int num_of_algo = 1;
    if (!updateInput(algorithm_path)) {
        fillSimErrors();
        err_detected = true;
        return false;
    }
    if (!dirExists(travels_dir_path)) {
        errors[0].push_back("@ FATAL ERROR: Can't find travel directory path.");
        fillSimErrors();
        err_detected = true;
        return false;
    }
    vector<string> algorithms = getSOFilesNames(algorithm_path);
    for (auto &algo_name_so : algorithms) {
        string algo_name = algo_name_so.substr(0, (int) algo_name_so.length() - 3);
        vector<string> travel_files;
        string plan_path, route_path;
        WeightBalanceCalculator calc;
        int num_of_errors = 0;
        int prev_size = (int) inst.algo_funcs.size();
        void *handler = dlopen((algorithm_path + std::filesystem::path::preferred_separator + algo_name_so).c_str(),
                               RTLD_LAZY);
        if (!validateAlgoLoad(handler, algo_name, prev_size)) {
            continue; // Algorithm loading failed, continue to the next algorithm.
        }
        std::unique_ptr<AbstractAlgorithm> algo = inst.algo_funcs[0].second();

        vector<string> new_res_row;
        statistics.push_back(new_res_row);
        statistics[num_of_algo].push_back(algo_name);
        vector<string> new_err_row;
        errors.push_back(new_err_row);
        errors[num_of_algo].push_back(algo_name);

        cout << "\nExecuting Algorithm " << algo_name << "..." << endl;
        bool empty_travel_dir = true; // Will become false once at least one folder found inside travels folder
        for (const auto &travel_dir : std::filesystem::directory_iterator(
                travels_dir_path)) { // Foreach Travel, do the following:
            if (!std::filesystem::is_directory(travel_dir))
                continue;
            empty_travel_dir = false;
            vector<pair<int, string>> errs_in_ctor;
            this->err_in_travel = false;
            this->curr_travel_name = travel_dir.path().filename();

            //Iterate over the directory
            if (!scanTravelDir(num_of_algo, plan_path, route_path, travel_files, travel_dir.path())) {
                continue; // Fatal error detected. Skip to the next travel.
            }

            if (num_of_algo == 1)
                statistics[0].push_back(travel_dir.path().filename()); // creating a travel column
            travel.initPortsContainersFiles(travel_dir.path(), travel_files, errs_in_ctor);
            //SIMULATION

            analyzeErrCode(algo->readShipPlan(plan_path), num_of_algo);
            analyzeErrCode(algo->readShipRoute(route_path), num_of_algo);
            analyzeErrCode(algo->setWeightBalanceCalculator(calc), num_of_algo);
            executeTravel(num_of_algo, algo_name, *algo, calc, errs_in_ctor, num_of_errors);
            if (num_of_algo == 1)
                extractGeneralErrors(errs_in_ctor); // Update the errors with general errors found during the travel.
            travel_files.clear();
        } // Done traveling
        if (empty_travel_dir) { // No travel dir found
            errors[0].push_back("@ FATAL ERROR: the given travels folder has no sub folders.");
            fillSimErrors();
            err_detected = true;
            return false;
        }
        if (num_of_algo == 1)
            statistics[0].push_back("Num Errors"); // creating a Num Errors column
        statistics[num_of_algo].push_back(to_string(num_of_errors));
        num_of_algo++;
        inst.algo_funcs.clear();
        algo.release(); // release in order to dlclose the handler
        dlclose(handler);
    } // Done algorithm
    if (err_detected) // Errors found, err_file should be created
        fillSimErrors();
    createResultsFile();
    return true; // No fatal errors were detected
}

void Simulator::extractGeneralErrors(vector<pair<int, string>> &err_strings) {
    if (!err_strings.empty())
        err_detected = true; //at least one error was found
    for (int i = 0; i < (int) err_strings.size(); ++i) {
        errors[0].push_back("@ Travel: " + curr_travel_name + "- " + err_strings[i].second);
    }
    err_strings.clear(); // Clearing the errors list for future re-use.
}

bool Simulator::validateInstruction(const vector<string> &instructions) { // Check if the text line is legal
    if (instructions.size() == 5) { // May be 'L' 'U' or 'R' instruction
        if (instructions[0] != "L" && instructions[0] != "U" && instructions[0] != "R")
            return false; //Invalid instruction
        if (!isNumber(instructions[2]) || !isNumber(instructions[3]) || !isNumber(instructions[4])) {
            return false;
        }
    } else if (instructions.size() == 8) {
        if (instructions[0] != "M") // check if it's really Move instruction
            return false;
        if (!isNumber(instructions[2]) || !isNumber(instructions[3]) || !isNumber(instructions[4]) ||
            !isNumber(instructions[5]) || !isNumber(instructions[6]) || !isNumber(instructions[7])) {
            return false;
        }
        // instructions[1] will be validated later.
    } else { // Wrong number of aguments for the instruction
        return false;
    }
    return true;
}

bool
Simulator::validateLoadOp(int num_of_algo, ShipPlan &ship, WeightBalanceCalculator &calc, int floor_num,
                          int x, int y, const Container *cont) {
    Spot *pos, *pos_below;
    // Spot validation
    if (!ship.spotInRange(x, y) || floor_num < 0 || floor_num >= ship.getNumOfDecks()) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container in Out-Of-Range position.");
        return false;
    }
    if (ship.isFull()) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container in a full ship.");
        return false; // Ship is full!
    }
    pos = &(ship.getSpotAt(floor_num, x, y));
    if (!pos->getAvailable() || pos->getContainer() != nullptr) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container in an unavailable spot.");
        return false;
    }
    //Container validation
    if (cont == nullptr || cont->getSpotInFloor() !=
                           nullptr) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load an unavailable container.");
        return false;
    }
    if (!travel.isInRoute(cont->getDestPort())) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container that its destination is not within the remaining route.");
        return false;
    }
    // Balance validation
    if (calc.tryOperation('L', cont->getWeight(), x, y) != WeightBalanceCalculator::APPROVED) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container that un-balances the ship.");
        return false;
    }
    if (floor_num != 0) {
        pos_below = &(ship.getSpotAt(floor_num - 1, x, y));
        if (pos_below->getAvailable() &&
            pos_below->getContainer() == nullptr) { // check if there is no container at the floor below
            errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                          "- Load a container in a spot that's above an empty spot.");
            return false;
        }
    }
    return true;
}

bool
Simulator::validateUnloadOp(int num_of_algo, ShipPlan &ship, WeightBalanceCalculator &calc, int floor_num,
                            int x, int y, const string &cont_id) {
    Spot *pos, *pos_above;
    // Spot validation
    if (!ship.spotInRange(x, y) || floor_num < 0 || floor_num >= ship.getNumOfDecks()) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Unload a container with ID: " + cont_id + "- from Out-Of-Range position.");
        return false;
    }
    pos = &(ship.getSpotAt(floor_num, x, y));
    if (!pos->getAvailable() || pos->getContainer() == nullptr) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Unload a container with ID: " + cont_id +
                                      "- from an unavailable or empty spot.");
        return false;
    }
    Container *cont = pos->getContainer();
    //Container validation
    if (cont_id != cont->getID()) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Unload a container with ID: " + cont_id + "- that isn't in the given spot.");
        return false;
    }
    // Balance validation
    if (calc.tryOperation('U', cont->getWeight(), x, y) != WeightBalanceCalculator::APPROVED) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Unload a container with ID: " + cont_id +
                                      "- from from the ship unbalance it.");
        return false;
    } else if (floor_num != ship.getNumOfDecks() - 1) {
        pos_above = &(ship.getSpotAt(floor_num + 1, x, y));
        if (pos_above->getContainer() != nullptr) { // check if there is a container at the floor above
            errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                          "- Unload a container with ID: " + cont_id +
                                          "- while there's a container above it.");
            return false;
        }
    }
    return true;
}

bool Simulator::validateMoveOp(int num_of_algo, ShipPlan &ship, WeightBalanceCalculator &calc,
                               int source_floor_num, int source_x, int source_y, int dest_floor_num, int dest_x,
                               int dest_y, const string &cont_id) {
    Spot *source_pos, *dest_pos, *pos_above, *pos_below;
    // Spots validation
    if (!ship.spotInRange(source_x, source_y) || source_floor_num < 0 || source_floor_num >= ship.getNumOfDecks() ||
        !ship.spotInRange(dest_x, dest_y) || dest_floor_num < 0 || dest_floor_num >= ship.getNumOfDecks()) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id + "- using Out-Of-Range position.");
        return false;
    }
    source_pos = &(ship.getSpotAt(source_floor_num, source_x, source_y));
    dest_pos = &(ship.getSpotAt(dest_floor_num, dest_x, dest_y));
    if (!source_pos->getAvailable() || source_pos->getContainer() == nullptr ||
        !dest_pos->getAvailable() || dest_pos->getContainer() != nullptr) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id + "- using unavailable spot.");
        return false;
    }
    Container *cont = source_pos->getContainer();
    //Container validation
    if (cont_id != cont->getID()) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id + "- that isn't in the given spot.");
        return false;
    }
    // Balance validation
    if (calc.tryOperation('U', cont->getWeight(), source_x, source_y) != WeightBalanceCalculator::APPROVED
        || calc.tryOperation('L', cont->getWeight(), dest_x, dest_y) != WeightBalanceCalculator::APPROVED) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id + "- cause the ship to unbalance.");
        return false;
    } else {
        if (source_floor_num != ship.getNumOfDecks() - 1) {
            pos_above = &(ship.getSpotAt(source_floor_num + 1, source_x, source_y));
            if (pos_above->getContainer() != nullptr) { // check if there is a container at the floor above
                errors[num_of_algo].push_back(
                        "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                        "- Move a container with ID: " + cont_id + "- while there's a container above it.");
                return false;
            }
            if (dest_floor_num != 0) {
                pos_below = &(ship.getSpotAt(dest_floor_num - 1, dest_x, dest_y));
                if (pos_below->getAvailable() &&
                    pos_below->getContainer() == nullptr) { // check if there is no container at the floor below
                    errors[num_of_algo].push_back(
                            "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                            "- Move a container with ID: " + cont_id + "- to a spot that's above an empty spot.");
                    return false;
                }
            }
        }
    }
    return true;
}

bool
Simulator::validateRejectOp(int num_of_algo, ShipPlan &ship, Route &travel,
                            int floor_num, int x, int y, const string &cont_id, bool &has_potential_to_be_loaded) {
    Container *cont;
    if (!Container::validateID(cont_id) || !Container::checkUnique(cont_id, false)) {
        return true; // Container got rejected cause of bad ID, which is legal!
    }
    cont = travel.getCurrentPort().getWaitingContainerByID(cont_id);
    //Container validation
    if (cont == nullptr) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Reject a container with ID: " + cont_id +
                                      "- that wasn't provided by the port.");
        return false; // Given id_cont is not in the waiting list
    }
    if (cont->getSpotInFloor() != nullptr) { // The container was loaded though reported rejected.
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Reject a container with ID: " + cont_id + "- that was already loaded.");
        return false;
    } else if (travel.isInRoute(cont->getDestPort()) && travel.getCurrentPort().getName() != cont->getDestPort()) {
        if (ship.getNumOfFreeSpots() > 0) {
            errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                          "- Reject a container with ID: " + cont_id +
                                          "- although it can be loaded correctly.");
            return false;
        }
        has_potential_to_be_loaded = true;
    }
    if (floor_num == x && x == y) {
        // Implement at ex.3: check floor_num, x and y values
    }
    return true;
}

void removeUnloadedContainer(map<string, Container *> &unloaded_containers, Container &cont) {
    if (unloaded_containers.find(cont.getID()) != unloaded_containers.end()) { // check if the container is at the map
        unloaded_containers.erase(cont.getID());
    }
}

int getFarthestDestOfContainerIndex(vector<Container> &conts) {
    int max_ind = -1;
    for (int i = 0; i < (int) conts.size(); ++i) {
        if (conts[i].getSpotInFloor() != nullptr) {                // find a container that was loaded on the ship
            max_ind = i; // i is always being raised
        }
    }
    return max_ind;
}

bool checkSortedContainers(vector<Container> &conts, Route &travel, const string &cont_id) {
    int farthest_port_num;
    travel.sortContainersByDestination(conts);
    farthest_port_num = getFarthestDestOfContainerIndex(
            conts); // get the maximal index of a container that was load to the ship.
    if (distance(conts.begin(), find(conts.begin(), conts.end(), *(Port::getContainerByIDFrom(conts, cont_id)))) <
        farthest_port_num) { // The left side of the statement is just to find cont_id's index in conts
        return false;
    }
    return true;
}

// Validates all the containers that were left at the port at the end of travel.
void Simulator::checkRemainingContainers(map<string, Container *> &unloaded_containers,
                                         map<string, Container *> &rejected_containers, Port &curr_port, Route &travel,
                                         int num_of_algo, int num_free_spots) {
    for (const auto &entry : unloaded_containers) {
        if (curr_port.getWaitingContainerByID(entry.first) != nullptr) {
            // In case the container was from the port
            if (rejected_containers.find(entry.first) !=
                rejected_containers.end()) { // check if the container was also rejected. if so, the container had a potential to be loaded on the ship.
                if (num_free_spots != 0) {
                    errors[num_of_algo].push_back(
                            "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                            "- Rejected a container with ID: " + entry.second->getID() +
                            "- although it can be loaded correctly.");
                    this->err_in_travel = true;
                } else if (!checkSortedContainers(curr_port.getWaitingContainers(), travel,
                                                  entry.first)) { // check if the container was rejected mistakenly
                    errors[num_of_algo].push_back(
                            "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                            "- Rejected a container with ID: " + entry.second->getID() +
                            "- while another container was loaded and it's destination port is further.");
                    this->err_in_travel = true;
                }
            } // <<< it is not possible to reach the else statement of that if
        } else { // In case the container was from the ship
            if (entry.second->getDestPort() != curr_port.getName()) { // The wrong container got unloaded!
                errors[num_of_algo].push_back(
                        "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                        "- A container with ID: " + entry.second->getID() +
                        "- was left in a port that's different from container's destination.");
                this->err_in_travel = true;
            }
        }
    }
}

void Simulator::checkPortContainers(set<string> &ignored_containers, int num_of_algo) {
    for (auto &container_id : ignored_containers) { // for each container that came from this port that was not treated.
        errors[num_of_algo].push_back(
                "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                "- A container with ID: " + container_id +
                "- was left at the port without getting an instruction.");
        this->err_in_travel = true;
    }
}

void Simulator::implementInstructions(WeightBalanceCalculator &calc, const string &instruction_file,
                                      int &num_of_operations, int num_of_algo) {
    FileHandler file(instruction_file);
    vector<string> instruction;
    Container *cont_to_load = nullptr;
    Port &current_port = travel.getCurrentPort();
    map<string, Container *> rejected_containers;
    map<string, Container *> unloaded_containers;
    set<string> ignoredContainers = current_port.getContainersIDFromPort();
    int x, y, floor_num;
    while (file.getNextLineAsTokens(instruction)) {
        if (!validateInstruction(instruction)) {
            errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                          "- Invalid instruction detected.");
            this->err_in_travel = true;
            continue;
        }
        if (ignoredContainers.find(instruction[1]) !=
            ignoredContainers.end()) { // Check if the container ID is from the port
            ignoredContainers.erase(instruction[1]);
        }
        AbstractAlgorithm::Action command = actionDic.at(instruction[0]);
        if (command != AbstractAlgorithm::Action::REJECT) {
            if (!Container::validateID(instruction[1])) {
                errors[num_of_algo].push_back(
                        "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                        "- Instruction with invalid container ID detected.");
                this->err_in_travel = true;
                continue; // Bad id for container
            }
            cont_to_load = current_port.getWaitingContainerByID(instruction[1]);
            if (cont_to_load ==
                nullptr) // didn't find the container on the waiting containers list, search in the reload list
                cont_to_load = (unloaded_containers.find(instruction[1]) != unloaded_containers.end())
                               ? unloaded_containers.at(instruction[1]) : nullptr;
        }
        floor_num = string2int(instruction[2]);
        x = string2int(instruction[3]);
        y = string2int(instruction[4]);
        switch (command) {
            case AbstractAlgorithm::Action::LOAD: {
                if (!validateLoadOp(num_of_algo, ship, calc, floor_num, x, y, cont_to_load)) {
                    this->err_in_travel = true;
                    break;
                }
                // Load container on the ship
                ship.insertContainer(floor_num, x, y, *cont_to_load);
                removeUnloadedContainer(unloaded_containers, *cont_to_load);
                num_of_operations++;
                break;
            }
            case AbstractAlgorithm::Action::UNLOAD: {
                if (!validateUnloadOp(num_of_algo, ship, calc, floor_num, x, y, instruction[1])) {
                    this->err_in_travel = true;
                    break;
                }
                // Unload container from the ship
                Container *temp_cont = ship.getContainerAt(floor_num, x,
                                                           y); // save pointer since removeContainer deletes it
                ship.removeContainer(floor_num, x, y);
                unloaded_containers.insert({temp_cont->getID(), temp_cont});
                num_of_operations++;
                break;
            }
            case AbstractAlgorithm::Action::MOVE: {
                if (!validateMoveOp(num_of_algo, ship, calc, floor_num, x, y, string2int(instruction[5]),
                                    string2int(instruction[6]), string2int(instruction[7]), instruction[1])) {
                    this->err_in_travel = true;
                    break;
                }
                // Move container on the ship
                ship.moveContainer(floor_num, x, y, string2int(instruction[5]), string2int(instruction[6]),
                                   string2int(instruction[7]));
                num_of_operations++;
                break;
            }
            case AbstractAlgorithm::Action::REJECT: {
                bool has_potential_to_be_loaded = false;
                if (!validateRejectOp(num_of_algo, ship, travel, floor_num, x, y, instruction[1],
                                      has_potential_to_be_loaded)) {
                    this->err_in_travel = true;
                    break;
                }
                Container *r_cont = current_port.getWaitingContainerByID(instruction[1]);
                rejected_containers.insert({instruction[1], r_cont});
                if (has_potential_to_be_loaded)
                    unloaded_containers.insert({instruction[1],
                                                r_cont}); // add to unloaded_containers so that we will later check if it was rejected correctly.
                break;
            }
            default: {
                errors[num_of_algo].push_back(
                        "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                        "- Invalid instruction detected.");
                this->err_in_travel = true;
            }
        }
    }
    checkRemainingContainers(unloaded_containers, rejected_containers, current_port, travel, num_of_algo,
                             ship.getNumOfFreeSpots());
    checkPortContainers(ignoredContainers, num_of_algo);
}

void Simulator::checkMissedContainers(const string &port_name, int num_of_algo) {
    vector<Container *> conts = ship.getContainersForDest(port_name);
    if ((int) conts.size() > 0) {
        errors[num_of_algo].push_back("@ Travel: " + this->curr_travel_name +
                                      "- There are some containers that were not unloaded at their destination port: " +
                                      port_name);
        this->err_detected = true;
    }
}

void Simulator::addSumColumn() {
    int sum;
    auto num_of_algos = (int) statistics.size();
    auto num_of_travels = (int) statistics[0].size() - 1; // -1 so we ignore the Errors column.
    statistics[0].insert(statistics[0].end() - 1, "Sum");
    for (int i = 1; i < num_of_algos; ++i) {
        sum = 0;
        for (int j = 1; j < num_of_travels; ++j) {
            if (statistics[i][j] == "-1") continue; // Ignore travels with errors
            sum += string2int(statistics[i][j]);
        }
        statistics[i].insert(statistics[i].end() - 1, to_string(sum));
    }
}

void Simulator::sortResults() {
    sort(statistics.begin(), statistics.end(), [](vector<string> &v1, vector<string> &v2) {
        if (v1[0] == "RESULTS") // Title row, always first
            return true;
        if (v2[0] == "RESULTS") // Title row, always first
            return false;
        int v1_errors = stoi(v1[(int) v1.size() - 1]);
        int v2_errors = stoi(v2[(int) v2.size() - 1]);
        if (v1_errors != v2_errors) // Sort by number of errors, if there is difference
            return v1_errors < v2_errors;
        // Sort by sum of actions in case of errors tie
        return stoi(v1[(int) v1.size() - 2]) < stoi(v2[(int) v2.size() - 2]);
    });
}

void Simulator::createResultsFile() {
    FileHandler res_file(this->output_dir_path + std::filesystem::path::preferred_separator + "simulation.results",
                         true);
    if (res_file.isFailed()) {
        return;
    }
    addSumColumn();
    sortResults();
    auto rows = (int) statistics.size();
    auto cols = (int) statistics[0].size();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (j == cols - 1) // in case we are in the last cell
                res_file.writeCell(statistics[i][j], true);
            else
                res_file.writeCell(statistics[i][j]);

        }
    }
}

void Simulator::fillSimErrors() {
    FileHandler err_file(this->output_dir_path + std::filesystem::path::preferred_separator + "simulation.errors",
                         true);
    if (err_file.isFailed()) {
        return;
    }
    for (int i = 0; i < (int) errors.size(); ++i) {
        if ((int) errors[i].size() == 1) {
            // No errors found for this algorithm, skip to the next one
            continue;
        }
        for (int j = 0; j < (int) errors[i].size(); ++j) {
            if (j == 0) // in case we are in the first cell
                err_file.writeCell("________________" + errors[i][j] + " Errors________________", true);
            else
                err_file.writeCell(errors[i][j], true);
        }
        err_file.writeCell("", true);
    }
}

void Simulator::printSimulationResults() {
    cout << "Results File:" << endl;
    printCSVFile(this->output_dir_path + std::filesystem::path::preferred_separator + "simulation.results");
}

void Simulator::printSimulationErrors() {
    if (!err_detected) return; // No errors were found
    cout << "Errors File:" << endl;
    if (!printCSVFile(this->output_dir_path + std::filesystem::path::preferred_separator + "simulation.errors"))
        cout << "Couldn't open errors file." << endl;
}

void Simulator::analyzeErrCode(int err_code, int num_of_algo) {
    vector<unsigned int> one_indexes = getOneIndexes(err_code);
    for (const unsigned int index : one_indexes) {
        if (index > 18) return; // No error code is defined for indexes above 18.
        errors[num_of_algo].push_back("@ Algorithm reported in travel " + curr_travel_name + ": " + errCodes.at(index));
    }
}