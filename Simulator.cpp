#include "Simulator.h"

Simulator::Simulator(const string &root) : rootDir(root), err_in_travel(false), err_detected(false) {
    vector<string> first_res_row;
    vector<string> first_err_row;

    statistics.push_back(first_res_row);
    statistics[0].push_back("RESULTS");
    errors.push_back(first_err_row);
    errors[0].push_back("General");
}

bool travelName(const string &name) {
    regex form("(travel)([0-9]+)");
    return regex_match(name, form);
}

// Checks if the file name is "travel*"
bool validateTravelFolder(std::filesystem::directory_entry entry) {
    if (std::filesystem::is_directory(entry.path()) && travelName(entry.path().filename())) {
        return true;
    }
    return false;
}

void Simulator::runSimulation() {
    int num_of_algo = 1;
    if (false) { // TODO: need to make general function to validate root directory
        cout << "ERROR: Can't find root directory path." << endl;
        return;
    }
    while (num_of_algo <= NUM_OF_ALGORITHMS) {
        vector<string> travel_files(1);
        bool successful_build = true;
        string plan_path, travel_path;
        WeightBalanceCalculator calc;
        int num_of_operations, num_of_errors = 0;
        Algorithm *algo;
        vector<string> new_res_row;
        statistics.push_back(new_res_row);
        statistics[num_of_algo].push_back("Algorithm ." + to_string(num_of_algo));
        vector<string> new_err_row;
        errors.push_back(new_err_row);
        errors[num_of_algo].push_back("Algorithm ." + to_string(num_of_algo));

        cout << "\nExecuting Algorithm no. " << num_of_algo << ":" << endl;
        for (const auto &travel_dir : std::filesystem::directory_iterator(
                this->rootDir)) { // Foreach Travel, do the following:
            num_of_operations = 0;
            if (!validateTravelFolder(travel_dir))
                continue;
            ShipPlan *ship = nullptr;
            Route *travel = nullptr;
            bool routeFound = false;
            bool planFound = false;
            this->err_in_travel = false;
            //Iterate over the directory
            for (const auto &entry : std::filesystem::directory_iterator(travel_dir.path())) {
                if (entry.path().filename() == "Plan.csv") { // A ship plan file was found
                    plan_path = entry.path();
                    ship = new ShipPlan(plan_path, successful_build);
                    planFound = true;
                } else if (entry.path().filename() == "Route.csv") { // A route file was found
                    travel_path = entry.path();
                    travel = new Route(travel_path);
                    routeFound = true;
                } else { // The rest of the files, may include some containers details.
                    travel_files.push_back(entry.path().filename());
                }
            }
            if (!planFound) {
                if (num_of_algo == 1) errors[0].push_back(""); // TODO: Insert the string into the cell
                err_detected = true;
                cout << "WARNING: Directory: " << travel_dir.path() << " is illegal, no Plan file" << endl;
                delete travel;
                continue;
            }
            if (!routeFound) {
                if (num_of_algo == 1) errors[0].push_back(""); // TODO: Insert the string into the cell
                err_detected = true;
                cout << "WARNING: Directory: " << travel_dir.path() << " is illegal, no Route file" << endl;
                delete ship;
                continue;
            }
            if (!successful_build) { // TODO: catch soft general errors of line that were ignored in the files, idea: make successful_build a pair
                if (num_of_algo == 1) errors[0].push_back(""); // TODO: Insert the string into the cell
                err_detected = true;
                continue; //One of the files of the travel is invalid, continue to the next travel.
            }
            if (num_of_algo == 1) statistics[0].push_back(travel_dir.path().filename()); // creating a travel column
            travel->initPortsContainersFiles(travel_dir.path(), travel_files);
            //SIMULATION
            switch (num_of_algo) {
                case 1:
                    algo = new Algorithm(*ship, travel, &calc);
                    break;
                case 2:
                    algo = new AlgorithmReverse(*ship, travel, &calc);
                    break;
            }
            string instruction_file;
            while (travel->moveToNextPort()) { // For each port in travel
                instruction_file = travel_dir.path().string();
                instruction_file = instruction_file + std::filesystem::path::preferred_separator + "instructions.csv";
                algo->getInstructionsForCargo(travel->getCurrentPort().getWaitingContainers(), instruction_file);
                this->implementInstructions(*ship, travel, calc, instruction_file, num_of_operations, num_of_algo);
                this->checkMissedContainers(ship, travel->getCurrentPort().getName());
                travel->clearCurrentPort();
            }
            Container::clearIDs(); // TODO: For each port in travel, we need to delete all the containers that were left at the port
            delete ship;
            ship = nullptr;
            delete travel;
            travel = nullptr;
            delete algo;
            algo = nullptr;
            travel_files.clear();
            FileHandler::deleteFile(instruction_file); // TODO: Save all crane_instructions files per port
            // Check if there was an error by the algorithm. if there was, number of operation is '-1'.
            if (this->err_in_travel) {
                err_detected = true;
                num_of_errors++;
                statistics[num_of_algo].push_back("-1");
            } else {
                statistics[num_of_algo].push_back(to_string(num_of_operations));
            }
        }
        if (num_of_algo == 1) statistics[0].push_back("Num Errors"); // creating a Num Errors column
        statistics[num_of_algo].push_back(to_string(num_of_errors));
        num_of_algo++;
    }
    if (err_detected) // Errors found, err_file should be created
        fillSimErrors();
}

// TODO: Check if this counts as an algorithm error
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
Simulator::validateLoadOp(int num_of_algo, ShipPlan &ship, const WeightBalanceCalculator &calc, int floor_num,
                          int x, int y, const Container *cont) {
    Spot *pos, *pos_below;
    // Spot validation
    if (!ship.spotInRange(x, y) || floor_num < 0 || floor_num >= ship.getNumOfDecks()) {
        errors[num_of_algo].push_back("Load a container in Out-Of-Range position.");
        return false;
    }
    if (ship.isFull()) {
        errors[num_of_algo].push_back("Load a container in a full ship.");
        return false; // Ship is full!
    }
    pos = ship.getSpotAt(floor_num, x, y);
    if (!pos->getAvailable() || pos->getContainer() != nullptr) {
        errors[num_of_algo].push_back("Load a container in an unavailable spot.");
        return false;
    }
    //Container validation
    if (cont == nullptr || cont->getSpotInFloor() !=
                           nullptr) { // check if the container was on the port's list of containers to load, or in the reload list
        errors[num_of_algo].push_back("Load an unavailable container.");
        return false;
    }
    if (!calc.weightCheck(ship, *cont)) {
        errors[num_of_algo].push_back("Load a container that's too heavy.");
        return false;
    }
    // Structural validation
    if (!calc.balanceTest(ship, *cont, *pos)) {
        errors[num_of_algo].push_back("Load a container that un-balances the ship.");
        return false;
    }
    if (floor_num != 0) {
        pos_below = ship.getSpotAt(floor_num - 1, x, y);
        if (pos_below->getAvailable() &&
            pos_below->getContainer() == nullptr) { // check if there is no container at the floor below
            errors[num_of_algo].push_back("Load a container in a spot that's above an empty spot.");
            return false;
        }
    }
    return true;
}

bool
Simulator::validateUnloadOp(int num_of_algo, ShipPlan &ship, const WeightBalanceCalculator &calc, int floor_num,
                            int x, int y, const string &cont_id) {
    Spot *pos, *pos_above;
    // Spot validation
    if (!ship.spotInRange(x, y) || floor_num < 0 || floor_num >= ship.getNumOfDecks()) {
        errors[num_of_algo].push_back("Unload a container from Out-Of-Range position.");
        return false;
    }
    pos = ship.getSpotAt(floor_num, x, y);
    if (!pos->getAvailable() || (pos->getAvailable() && pos->getContainer() == nullptr)) {
        errors[num_of_algo].push_back("Unload a container from an unavailable or empty spot.");
        return false;
    }
    Container *cont = pos->getContainer();
    //Container validation
    if (cont_id != cont->getID()) {
        errors[num_of_algo].push_back("Unload a container that isn't in the given spot.");
        return false;
    }
    // Structural validation
    if (!calc.balanceTest(ship, *cont, *pos)) {
        errors[num_of_algo].push_back("Unload a container from from the ship unbalance it.");
        return false;
    } else if (floor_num != ship.getNumOfDecks() - 1) {
        pos_above = ship.getSpotAt(floor_num + 1, x, y);
        if (pos_above->getContainer() != nullptr) { // check if there is a container at the floor above
            errors[num_of_algo].push_back("Unload a container while there's a container above it.");
            return false;
        }
    }
    return true;
}

bool Simulator::validateMoveOp(int num_of_algo, ShipPlan &ship, const WeightBalanceCalculator &calc,
                               int source_floor_num, int source_x, int source_y, int dest_floor_num, int dest_x,
                               int dest_y, const string &cont_id) {
    Spot *source_pos, *dest_pos, *pos_above, *pos_below;
    // Spots validation
    if (!ship.spotInRange(source_x, source_y) || source_floor_num < 0 || source_floor_num >= ship.getNumOfDecks() ||
        !ship.spotInRange(dest_x, dest_y) || dest_floor_num < 0 || dest_floor_num >= ship.getNumOfDecks()) {
        errors[num_of_algo].push_back("Move a container using Out-Of-Range position.");
        return false;
    }
    source_pos = ship.getSpotAt(source_floor_num, source_x, source_y);
    dest_pos = ship.getSpotAt(dest_floor_num, dest_x, dest_y);
    if (source_pos->getAvailable() || source_pos->getContainer() == nullptr ||
        dest_pos->getAvailable() || dest_pos->getContainer() != nullptr) {
        errors[num_of_algo].push_back("Move a container using unavailable spot.");
        return false;
    }
    Container *cont = source_pos->getContainer();
    //Container validation
    if (cont_id != cont->getID()) {
        errors[num_of_algo].push_back("Move a container that isn't in the given spot.");
        return false;
    }
    // Structural validation
    if (!calc.balanceTest(ship, *cont, *dest_pos)) {
        errors[num_of_algo].push_back("Move a container cause the ship to unbalance.");
        return false;
    } else {
        if (source_floor_num != ship.getNumOfDecks() - 1) {
            pos_above = ship.getSpotAt(source_floor_num + 1, source_x, source_y);
            if (pos_above->getContainer() != nullptr) { // check if there is a container at the floor above
                errors[num_of_algo].push_back("Move a container while there's a container above it.");
                return false;
            }
            if (dest_floor_num != 0) {
                pos_below = ship.getSpotAt(dest_floor_num - 1, dest_x, dest_y);
                if (pos_below->getAvailable() &&
                    pos_below->getContainer() == nullptr) { // check if there is no container at the floor below
                    errors[num_of_algo].push_back("Move a container to a spot that's above an empty spot.");
                    return false;
                }
            }
        }
    }
    return true;
}

bool
Simulator::validateRejectOp(int num_of_algo, ShipPlan &ship, Route *travel, const WeightBalanceCalculator &calc,
                            int floor_num, int x, int y, const string &cont_id, bool &has_potential_to_be_loaded) {
    Container *cont;
    if (!Container::validateID(cont_id, false)) {
        return true; // Container got rejected cause of bad ID, which is legal!
    }
    cont = travel->getCurrentPort().getWaitingContainerByID(cont_id);
    //Container validation
    if (cont == nullptr) {
        errors[num_of_algo].push_back("Reject a container that wasn't provided by the port.");
        return false; // Given id_cont is not in the waiting list
    }
    if (cont->getSpotInFloor() != nullptr) { // The container was loaded though reported rejected.
        errors[num_of_algo].push_back("Reject a container that was already loaded.");
        return false;
    } else if (calc.weightCheck(ship, *cont) && travel->isInRoute(cont->getDestPort()) &&
               travel->getCurrentPort().getName() != cont->getDestPort()) {
        if (ship.getNumOfFreeSpots() > 0) {
            errors[num_of_algo].push_back("Reject a container although it can be loaded correctly.");
            return false;
        }
        has_potential_to_be_loaded = true;
    }
    if (floor_num == x && x == y) {
        // Implement at ex.2: check floor_num, x and y values
    }
    return true;
}

void removeUnloadedContainer(map<string, Container *> &unloaded_containers, Container &cont) {
    if (unloaded_containers.find(cont.getID()) != unloaded_containers.end()) { // check if the container is at the map
        unloaded_containers.erase(cont.getID());
    }
}

int getFarthestDestOfContainerIndex(vector<Container *> conts) {
    int max_ind = -1;
    for (int i = 0; i < (int) conts.size(); ++i) {
        if (conts[i]->getSpotInFloor() != nullptr) {                // find a container that was loaded on the ship
            max_ind = i; // i is always being raised
        }
    }
    return max_ind;
}

bool checkSortedContainers(vector<Container *> conts, Route *travel, const string &cont_id) {
    int farthest_port_num;
    travel->sortContainersByDestination(conts);
    farthest_port_num = getFarthestDestOfContainerIndex(
            conts); // get the maximal index of a container that was load to the ship.
    if (distance(conts.begin(), find(conts.begin(), conts.end(), Port::getContainerByIDFrom(conts, cont_id))) <
        farthest_port_num) { // The left side of the statement is just to find cont_id's index in conts
        return false;
    }
    return true;
}

void Simulator::checkRemainingContainers(map<string, Container *> unloaded_containers,
                                         map<string, Container *> rejected_containers, Port &curr_port, Route *travel,
                                         int num_of_algo, int num_free_spots) {
    for (const auto &entry : unloaded_containers) {
        if (curr_port.getWaitingContainerByID(entry.first) != nullptr) {
            // In case the container was from the port
            if (rejected_containers.find(entry.first) !=
                rejected_containers.end()) { // check if the container was also rejected
                if (num_free_spots != 0) {
                    errors[num_of_algo].push_back("Rejected a container although it can be loaded correctly.");
                } else if (!checkSortedContainers(curr_port.getWaitingContainers(), travel,
                                                  entry.first)) { // check if the container was rejected mistakenly
                    errors[num_of_algo].push_back(
                            "Rejected a container while another container was loaded and it's destination port is further.");
                }
            } // <<< it is not possible to reach the else statement of that if
        } else { // In case the container was from the ship
            if (entry.second->getDestPort() != curr_port.getName()) { // The wrong container got unloaded!
                errors[num_of_algo].push_back(
                        "A container was left in a port that's different from container's destination.");
            }
        }
    }
}

// Deletes only the containers that was unloaded from ship
void deleteRemainingContainers(map<string, Container *> &unloaded_containers,
                               map<string, Container *> &rejected_containers) {
    for (auto &entry : unloaded_containers) {
        if (rejected_containers.find(entry.first) != rejected_containers.end()) {
            continue; // found an unloaded container which was provided by the port
        }
        delete entry.second; // Delete the container it FOREVER.
    }
    unloaded_containers.clear();
}

void Simulator::implementInstructions(ShipPlan &ship, Route *travel,
                                      const WeightBalanceCalculator &calc, const string &instruction_file,
                                      int &num_of_operations, int num_of_algo) {
    FileHandler file(instruction_file);
    vector<string> instruction;
    Container *cont_to_load;
    Port *current_port = &(travel->getCurrentPort());
    map<string, Container *> rejected_containers;
    map<string, Container *> unloaded_containers;
    int x, y, floor_num;
    while (file.getNextLineAsTokens(instruction)) {
        if (!validateInstruction(instruction)) {
            cout << "WARNING: Skipping an invalid instruction, continue to the next one." << endl;
            continue;
        }
        iType command = Algorithm::dic.at(instruction[0]);
        if (command != R) {
            if (!Container::validateID(instruction[1], false)) {
                cout << "WARNING: Skipping an invalid instruction, continue to the next one." << endl;
                continue; // Bad id for container
            }
            cont_to_load = current_port->getWaitingContainerByID(instruction[1]);
            if (cont_to_load ==
                nullptr) // didn't find the container on the waiting containers list, search in the reload list
                cont_to_load = (unloaded_containers.find(instruction[1]) != unloaded_containers.end())
                               ? unloaded_containers.at(instruction[1]) : nullptr;
        }
        floor_num = string2int(instruction[2]);
        x = string2int(instruction[3]);
        y = string2int(instruction[4]);
        switch (command) {
            case L: {
                if (!validateLoadOp(num_of_algo, ship, calc, floor_num, x, y, cont_to_load)) {
                    this->err_in_travel = true;
                    return;
                    // break; //Will be used in ex.2
                }
                // Load container on the ship
                ship.insertContainer(floor_num, x, y, *cont_to_load);
                removeUnloadedContainer(unloaded_containers, *cont_to_load);
                num_of_operations++;
                break;
            }
            case U: {
                if (!validateUnloadOp(num_of_algo, ship, calc, floor_num, x, y, instruction[1])) {
                    this->err_in_travel = true;
                    return;
                    // break; //Will be used in ex.2
                }
                // Unload container from the ship
                Container *temp_cont = ship.getContainerAt(floor_num, x,
                                                           y); // save pointer since removeContainer deletes it
                ship.removeContainer(floor_num, x, y);
                unloaded_containers.insert({temp_cont->getID(), temp_cont});
                num_of_operations++;
                break;
            }
            case M: {
                if (!validateMoveOp(num_of_algo, ship, calc, floor_num, x, y, string2int(instruction[5]),
                                    string2int(instruction[6]), string2int(instruction[7]), instruction[1])) {
                    this->err_in_travel = true;
                    return;
                    // break; //Will be used in ex.2
                }
                // Move container on the ship
                ship.moveContainer(floor_num, x, y, string2int(instruction[5]), string2int(instruction[6]),
                                   string2int(instruction[7]));
                num_of_operations++;
                break;
            }
            case R: {
                bool has_potential_to_be_loaded = false;
                if (!validateRejectOp(num_of_algo, ship, travel, calc, floor_num, x, y, instruction[1],
                                      has_potential_to_be_loaded)) {
                    this->err_in_travel = true;
                    return;
                    // break; //Will be used in ex.2
                }
                Container *r_cont = current_port->getWaitingContainerByID(instruction[1]);
                rejected_containers.insert({instruction[1], r_cont});
                if (has_potential_to_be_loaded)
                    unloaded_containers.insert({instruction[1],
                                                r_cont}); // add to unloaded_containers so that we will later check if it was rejected correctly.
                break;
            }
            default: {
                cout << "WARNING: Skipping an invalid instruction, continue to the next one." << endl;
            }
        }
    }
    checkRemainingContainers(unloaded_containers, rejected_containers, *current_port, travel, num_of_algo,
                             ship.getNumOfFreeSpots());
    // Delete the remaining containers at unloaded+rejected.
    deleteRemainingContainers(unloaded_containers, rejected_containers);
}

void Simulator::checkMissedContainers(ShipPlan *ship, const string &port_name) {
    vector<Container *> conts = ship->getContainersForDest(port_name);
    if ((int) conts.size() > 0) { // TODO: Should this be an algorithm error?
        cout << "WARNING: There are some containers that were not unloaded at their destination port." << endl;
    }
}

void Simulator::addSumColumn() {
    int sum;
    int num_of_algos = (int) statistics.size();
    int num_of_travels = (int) statistics[0].size() - 1; // -1 so we ignore the Errors column.
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

void Simulator::createResultsFile() {
    FileHandler res_file(this->rootDir + std::filesystem::path::preferred_separator + "simulation.results.csv", true);
    if (res_file.isFailed()) {
        return;
    }
    addSumColumn();
    int rows = (int) statistics.size();
    int cols = (int) statistics[0].size();
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            res_file.writeCell(statistics[i][j]);
        }
        res_file.writeCell("", true);
    }
}

void Simulator::fillSimErrors() {
    FileHandler err_file(this->rootDir + std::filesystem::path::preferred_separator + "simulation.errors.csv", true);
    if (err_file.isFailed()) {
        return;
    }
    for (int i = 0; i < (int) errors.size(); ++i) {
        if ((int) errors[i].size() == 1) {
            // No errors found for this algorithm, skip to the next one
            continue;
        }
        for (int j = 0; j < (int) errors[i].size(); ++j) {
            err_file.writeCell(errors[i][j]);
        }
        err_file.writeCell("", true);
    }
}
