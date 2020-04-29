#include "Simulator.h"

Simulator::Simulator(const string &root) : rootDir(root), end_travel(false) {
    vector<string> row;
    statistics.push_back(row);
    statistics[0].push_back("RESULTS");
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
    FileHandler err_file(this->rootDir + std::filesystem::path::preferred_separator + "simulation.errors.csv", true);
    if(err_file.isFailed()){
        cout << "ERROR: Can't find root directory path." << endl;
        return;
    }
    while (num_of_algo <= NUM_OF_ALGORITHMS) {
        vector<string> travel_files(1);
        bool successful_build = true;
        string plan_path, travel_path;
        WeightBalanceCalculator calc;
        int num_of_operations;
        Algorithm *algo;

        err_file.writeCell("Algorithm ." + to_string(num_of_algo));
        vector<string> new_res_row;
        statistics.push_back(new_res_row);
        statistics[num_of_algo].push_back("Algorithm ." + to_string(num_of_algo));

        cout << "\nExecuting Algorithm no. " << num_of_algo << ":" << endl;
        for (const auto &travel_dir : std::filesystem::directory_iterator(
                this->rootDir)) { // Foreach Travel, do the following:
            num_of_operations = 0;
            if (!validateTravelFolder(travel_dir))
                continue;
            if (num_of_algo == 1) statistics[0].push_back(travel_dir.path().filename()); // creating a travel column
            ShipPlan *ship = nullptr;
            Route *travel = nullptr;
            bool routeFound = false;
            bool planFound = false;
            this->end_travel = false;
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
                cout << "WARNING: Directory: " << travel_dir.path() << " is illegal, no Plan file" << endl;
                statistics[num_of_algo].push_back(to_string(num_of_operations));
                delete travel;
                continue;
            }
            if (!routeFound) {
                cout << "WARNING: Directory: " << travel_dir.path() << " is illegal, no Route file" << endl;
                statistics[num_of_algo].push_back(to_string(num_of_operations));
                delete ship;
                continue;
            }
            if (!successful_build) {
                statistics[num_of_algo].push_back(to_string(num_of_operations));
                continue; //One of the files of the travel is invalid, continue to the next travel.
            }
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
            while (!this->end_travel && travel->moveToNextPort()) { // For each port in travel
                instruction_file = travel_dir.path().string();
                instruction_file = instruction_file + std::filesystem::path::preferred_separator + "instructions.csv";
                algo->getInstructionsForCargo(travel->getCurrentPort().getWaitingContainers(), instruction_file);
                this->implementInstructions(err_file, *ship, travel, calc, instruction_file, num_of_operations);
                this->checkMissedContainers(ship, travel->getCurrentPort().getName());
                travel->clearCurrentPort();
            }
            Container::clearIDs();
            delete ship;
            ship = nullptr;
            delete travel;
            travel = nullptr;
            delete algo;
            algo = nullptr;
            travel_files.clear();
            FileHandler::deleteFile(instruction_file);
            statistics[num_of_algo].push_back(to_string(num_of_operations));
        }
        err_file.writeCell("", true);
        num_of_algo++;
    }
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
Simulator::validateLoadOp(FileHandler &err_file, ShipPlan &ship, const WeightBalanceCalculator &calc, int floor_num,
                          int x, int y, const Container *cont) {
    Spot *pos, *pos_below;
    // Spot validation
    if (!ship.spotInRange(x, y) || floor_num < 0 || floor_num >= ship.getNumOfDecks()) {
        err_file.writeCell("Load a container in Out-Of-Range position.");
        return false;
    }
    if (ship.isFull()) {
        err_file.writeCell("Load a container in a full ship.");
        return false; // Ship is full!
    }
    pos = ship.getSpotAt(floor_num, x, y);
    if (!pos->getAvailable() || pos->getContainer() != nullptr) {
        err_file.writeCell("Load a container in an unavailable spot.");
        return false;
    }
    //Container validation
    if (cont == nullptr || cont->getSpotInFloor() !=
                           nullptr) { // check if the container was on the port's list of containers to load, or in the reload list
        err_file.writeCell("Load an unavailable container.");
        return false;
    }
    if (!calc.weightCheck(ship, *cont)) {
        err_file.writeCell("Load a container that's too heavy.");
        return false;
    }
    // Structural validation
    if (!calc.balanceTest(ship, *cont, *pos)) {
        err_file.writeCell("Load a container that un-balances the ship.");
        return false;
    }
    if (floor_num != 0) {
        pos_below = ship.getSpotAt(floor_num - 1, x, y);
        if (pos_below->getAvailable() &&
            pos_below->getContainer() == nullptr) { // check if there is no container at the floor below
            err_file.writeCell("Load a container in a spot that's above an empty spot.");
            return false;
        }
    }
    return true;
}

bool
Simulator::validateUnloadOp(FileHandler &err_file, ShipPlan &ship, const WeightBalanceCalculator &calc, int floor_num,
                            int x, int y, const string &cont_id) {
    Spot *pos, *pos_above;
    // Spot validation
    if (!ship.spotInRange(x, y) || floor_num < 0 || floor_num >= ship.getNumOfDecks()) {
        err_file.writeCell("Unload a container from Out-Of-Range position.");
        return false;
    }
    pos = ship.getSpotAt(floor_num, x, y);
    if (!pos->getAvailable() || (pos->getAvailable() && pos->getContainer() == nullptr)) {
        err_file.writeCell("Unload a container from an unavailable or empty spot.");
        return false;
    }
    Container *cont = pos->getContainer();
    //Container validation
    if (cont_id != cont->getID()) {
        err_file.writeCell("Unload a container that isn't in the given spot.");
        return false;
    }
    // Structural validation
    if (!calc.balanceTest(ship, *cont, *pos)) {
        err_file.writeCell("Unload a container from from the ship unbalance it.");
        return false;
    } else if (floor_num != ship.getNumOfDecks() - 1) {
        pos_above = ship.getSpotAt(floor_num + 1, x, y);
        if (pos_above->getContainer() != nullptr) { // check if there is a container at the floor above
            err_file.writeCell("Unload a container while there's a container above it.");
            return false;
        }
    }
    return true;
}

bool Simulator::validateMoveOp(FileHandler &err_file, ShipPlan &ship, const WeightBalanceCalculator &calc,
                               int source_floor_num, int source_x, int source_y, int dest_floor_num, int dest_x,
                               int dest_y, const string &cont_id) {
    Spot *source_pos, *dest_pos, *pos_above, *pos_below;
    // Spots validation
    if (!ship.spotInRange(source_x, source_y) || source_floor_num < 0 || source_floor_num >= ship.getNumOfDecks() ||
        !ship.spotInRange(dest_x, dest_y) || dest_floor_num < 0 || dest_floor_num >= ship.getNumOfDecks()) {
        err_file.writeCell("Move a container using Out-Of-Range position.");
        return false;
    }
    source_pos = ship.getSpotAt(source_floor_num, source_x, source_y);
    dest_pos = ship.getSpotAt(dest_floor_num, dest_x, dest_y);
    if (source_pos->getAvailable() || source_pos->getContainer() == nullptr ||
        dest_pos->getAvailable() || dest_pos->getContainer() != nullptr) {
        err_file.writeCell("Move a container using unavailable spot.");
        return false;
    }
    Container *cont = source_pos->getContainer();
    //Container validation
    if (cont_id != cont->getID()) {
        err_file.writeCell("Move a container that isn't in the given spot.");
        return false;
    }
    // Structural validation
    if (!calc.balanceTest(ship, *cont, *dest_pos)) {
        err_file.writeCell("Move a container cause the ship to unbalance.");
        return false;
    } else {
        if (source_floor_num != ship.getNumOfDecks() - 1) {
            pos_above = ship.getSpotAt(source_floor_num + 1, source_x, source_y);
            if (pos_above->getContainer() != nullptr) { // check if there is a container at the floor above
                err_file.writeCell("Move a container while there's a container above it.");
                return false;
            }
            if (dest_floor_num != 0) {
                pos_below = ship.getSpotAt(dest_floor_num - 1, dest_x, dest_y);
                if (pos_below->getAvailable() &&
                    pos_below->getContainer() == nullptr) { // check if there is no container at the floor below
                    err_file.writeCell("Move a container to a spot that's above an empty spot.");
                    return false;
                }
            }
        }
    }
    return true;
}

bool Simulator::validateRejectOp(FileHandler &err_file, ShipPlan &ship, Route *travel, const WeightBalanceCalculator &calc,
                            int floor_num, int x, int y, const string &cont_id, bool &has_potential_to_be_loaded) {
    Container *cont;
    if (!Container::validateID(cont_id, false)) {
        return true; // Container got rejected cause of bad ID, which is legal!
    }
    cont = travel->getCurrentPort().getWaitingContainerByID(cont_id);
    //Container validation
    if (cont == nullptr) {
        err_file.writeCell("Reject a container that wasn't provided by the port.");
        return false; // Given id_cont is not in the waiting list
    }
    if (cont->getSpotInFloor() != nullptr) { // The container was loaded though reported rejected.
        err_file.writeCell("Reject a container that was already loaded.");
        return false;
    } else if (calc.weightCheck(ship, *cont) && travel->isInRoute(cont->getDestPort()) &&
               travel->getCurrentPort().getName() != cont->getDestPort()) {
        if (ship.getNumOfFreeSpots() > 0) {
            err_file.writeCell("Reject a container although it can be loaded correctly.");
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
                                         FileHandler &err_file, int num_free_spots) {
    for (const auto &entry : unloaded_containers) {
        if (curr_port.getWaitingContainerByID(entry.first) != nullptr) {
            // In case the container was from the port
            if (rejected_containers.find(entry.first) !=
                rejected_containers.end()) { // check if the container was also rejected
                if (num_free_spots != 0) {
                    err_file.writeCell("Rejected a container although it can be loaded correctly.");
                } else if (!checkSortedContainers(curr_port.getWaitingContainers(), travel,
                                                  entry.first)) { // check if the container was rejected mistakenly
                    err_file.writeCell(
                            "Rejected a container while another container was loaded and it's destination port is further.");
                }
            } // <<< it is not possible to reach the else statement of that if
        } else { // In case the container was from the ship
            if (entry.second->getDestPort() != curr_port.getName()) { // The wrong container got unloaded!
                err_file.writeCell("A container was left in a port that's different from container's destination.");
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

void Simulator::implementInstructions(FileHandler &err_file, ShipPlan &ship, Route *travel,
                                      const WeightBalanceCalculator &calc, const string &instruction_file,
                                      int &num_of_operations) {
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
                if (!validateLoadOp(err_file, ship, calc, floor_num, x, y, cont_to_load)) {
                    this->end_travel = true;
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
                if (!validateUnloadOp(err_file, ship, calc, floor_num, x, y, instruction[1])) {
                    this->end_travel = true;
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
                if (!validateMoveOp(err_file, ship, calc, floor_num, x, y, string2int(instruction[5]),
                                    string2int(instruction[6]), string2int(instruction[7]), instruction[1])) {
                    this->end_travel = true;
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
                if (!validateRejectOp(err_file, ship, travel, calc, floor_num, x, y, instruction[1], has_potential_to_be_loaded)) {
                    this->end_travel = true;
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
    checkRemainingContainers(unloaded_containers, rejected_containers, *current_port, travel, err_file,
                             ship.getNumOfFreeSpots());
    // Delete the remaining containers at unloaded+rejected.
    deleteRemainingContainers(unloaded_containers, rejected_containers);
}

void Simulator::checkMissedContainers(ShipPlan *ship, const string &port_name) {
    vector<Container *> conts = ship->getContainersForDest(port_name);
    if ((int) conts.size() > 0) {
        cout << "WARNING: There are some containers that were not unloaded at their destination port." << endl;
    }
}

void Simulator::addSumColumn() {
    int sum;
    int num_of_algos = (int) statistics.size();
    int num_of_travels = (int) statistics[0].size();
    statistics[0].push_back("Sum");
    for (int i = 1; i < num_of_algos; ++i) {
        sum = 0;
        for (int j = 1; j < num_of_travels; ++j) {
            sum += string2int(statistics[i][j]);
        }
        statistics[i].push_back(to_string(sum));
    }
}

void Simulator::createResultsFile() {
    FileHandler res_file(this->rootDir + std::filesystem::path::preferred_separator + "simulation.results.csv", true);
    if(res_file.isFailed()){
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
