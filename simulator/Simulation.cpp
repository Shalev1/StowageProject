#include "Simulation.h"

enum InstructionType {
    Command = 0, ContainerID = 1, FloorNum = 2, X = 3, Y = 4, DestFloorNum = 5, DestX = 6, DestY = 7
};

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


Simulation::Simulation(ShipPlan &plan, Route &route, WeightBalanceCalculator &wcalc, int num_of_algo, int num_of_travel, string &travel_name, const string &output_path)
        : ship(plan), travel(route), calc(wcalc), err_in_travel(false), curr_travel_name(travel_name), output_dir_path(output_path), num_of_algo(num_of_algo), num_of_travel(num_of_travel) {
}

bool Simulation::executeTravel(const string &algo_name, AbstractAlgorithm &algo,
                               WeightBalanceCalculator &calc, int &num_of_errors) {
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
    while (travel.moveToNextPort()) { // For each port in travel
        curr_port_name = travel.getCurrentPort().getName();
        instruction_file =
                instruction_file_path + std::filesystem::path::preferred_separator + curr_port_name + "_" +
                to_string(travel.getNumOfVisitsInPort(curr_port_name)) + ".crane_instructions";
        analyzeErrCode(algo.getInstructionsForCargo(travel.getCurrentPortPath(), instruction_file));
        iterateInstructions(calc, instruction_file, num_of_operations, num_of_algo);
        checkMissedContainers(travel.getCurrentPort().getName());
    }
    // Check if there was an error by the algorithm. if there was, number of operation is '-1'.
    if (this->err_in_travel) {
        num_of_errors++;
        Simulator::insertResult(num_of_algo, num_of_travel,"-1", true);
        return false;
    }
    Simulator::insertResult(num_of_algo, num_of_travel,to_string(num_of_operations), false);
    return true;
}


bool
Simulation::runSimulation(pair<string, std::function<std::unique_ptr<AbstractAlgorithm>()>> algo_p, string &plan_path,
                          string &route_path) {
    vector<string> travel_files;
    int num_of_errors = 0;
    bool no_errors_detected;
    vector<pair<int, string>> errs_in_ctor;
    std::unique_ptr<AbstractAlgorithm> algo = algo_p.second();

    cout << "\nExecuting Travel " << curr_travel_name << "..." << endl;
    //SIMULATION
    analyzeErrCode(algo->readShipPlan(plan_path));
    analyzeErrCode(algo->readShipRoute(route_path));
    analyzeErrCode(algo->setWeightBalanceCalculator(calc));

    no_errors_detected = executeTravel(algo_p.first, *algo, calc, num_of_errors);

    return no_errors_detected; // true if no errors were detected.
}

bool Simulation::validateInstruction(const vector<string> &instructions) { // Check if the text line is legal
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

void Simulation::reportInvalidContainer(Container *cont) {
    // Containers ID is validated earlier.
    if (cont->getWeight() <= 0) {
        Simulator::insertError(num_of_algo, num_of_travel, "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Trying to load a container with illegal weight: " +
                                      to_string(cont->getWeight()));
    } else if (!Port::validateName(cont->getDestPort())) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Trying to load a container with illegal destination port: " +
                                      cont->getDestPort());
    } else if (ship.isContOnShip(cont->getID())) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Trying to load a container which it's ID already exists on the ship: " +
                                      cont->getID());
    }
    // DEBUG:Should never reach here.
    Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                  "- Trying to load an invalid container.");
}

bool
Simulation::validateLoadOp(Port &curr_port, WeightBalanceCalculator &calc,
                           int floor_num,
                           int x, int y, Container *cont) {
    Spot *pos, *pos_below;
    // Spot validation
    if (!ship.spotInRange(x, y) || floor_num < 0 || floor_num >= ship.getNumOfDecks()) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container in Out-Of-Range position.");
        return false;
    }
    if (ship.isFull()) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container in a full ship.");
        return false; // Ship is full!
    }
    pos = &(ship.getSpotAt(floor_num, x, y));
    if (!pos->getAvailable() || pos->getContainer() != nullptr) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container in an unavailable spot.");
        return false;
    }
    //Container validation
    if (cont == nullptr) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Trying to load an unavailable container.");
        return false; // Given id_cont is not in the waiting list
    }
    if (!cont->isValid()) { // Check if the container is not valid
        reportInvalidContainer(cont);
        return false;
    } else { // Container is valid, now check the duplication case
        if (cont->getSpotInFloor() != nullptr && curr_port.getNumOfDuplicates(cont->getID()) > 0) {
            Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                          "- Trying to load a container with a duplicated ID: " + cont->getID());
            curr_port.decreaseDuplicateId(cont->getID()); // Update that a duplicated ID container got treated
            return false;
        }
    }
    if (cont->getSpotInFloor() != nullptr) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Trying to load a container that is already on the ship.");
        return false;
    }
    if (cont->getDestPort() == this->curr_port_name) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container that its destination is the current port.");
        return false;
    }
    if (!travel.isInRoute(cont->getDestPort())) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container that its destination is not within the remaining route.");
        return false;
    }
    // Balance validation
    if (calc.tryOperation('L', cont->getWeight(), x, y) != WeightBalanceCalculator::APPROVED) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Load a container that un-balances the ship.");
        return false;
    }
    if (floor_num != 0) {
        pos_below = &(ship.getSpotAt(floor_num - 1, x, y));
        if (pos_below->getAvailable() &&
            pos_below->getContainer() == nullptr) { // check if there is no container at the floor below
            Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                          "- Load a container in a spot that's above an empty spot.");
            return false;
        }
    }
    return true;
}

bool
Simulation::validateUnloadOp(WeightBalanceCalculator &calc, int floor_num,
                             int x, int y, const string &cont_id) {
    Spot *pos, *pos_above;
    // Spot validation
    if (!ship.spotInRange(x, y) || floor_num < 0 || floor_num >= ship.getNumOfDecks()) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Unload a container with ID: " + cont_id + "- from Out-Of-Range position.");
        return false;
    }
    pos = &(ship.getSpotAt(floor_num, x, y));
    if (!pos->getAvailable() || pos->getContainer() == nullptr) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Unload a container with ID: " + cont_id +
                                      "- from an unavailable or empty spot.");
        return false;
    }
    Container *cont = pos->getContainer();
    //Container validation
    if (cont_id != cont->getID()) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Unload a container with ID: " + cont_id + "- that isn't in the given spot.");
        return false;
    }
    // Balance validation
    if (calc.tryOperation('U', cont->getWeight(), x, y) != WeightBalanceCalculator::APPROVED) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Unload a container with ID: " + cont_id +
                                      "- from from the ship unbalance it.");
        return false;
    } else if (floor_num != ship.getNumOfDecks() - 1) {
        pos_above = &(ship.getSpotAt(floor_num + 1, x, y));
        if (pos_above->getContainer() != nullptr) { // check if there is a container at the floor above
            Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                          "- Unload a container with ID: " + cont_id +
                                          "- while there's a container above it.");
            return false;
        }
    }
    return true;
}

bool Simulation::validateMoveOp(WeightBalanceCalculator &calc,
                                int source_floor_num, int source_x, int source_y, int dest_floor_num, int dest_x,
                                int dest_y, const string &cont_id) {
    Spot *source_pos, *dest_pos, *pos_above, *pos_below;
    // Spots validation
    if (!ship.spotInRange(source_x, source_y) || source_floor_num < 0 || source_floor_num >= ship.getNumOfDecks() ||
        !ship.spotInRange(dest_x, dest_y) || dest_floor_num < 0 || dest_floor_num >= ship.getNumOfDecks()) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id + "- using Out-Of-Range position.");
        return false;
    }
    if ((source_x == dest_x) && (source_y == dest_y) && (source_floor_num != dest_floor_num)) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id +
                                      "- to a spot with the same X,Y but at different floor.");
        return false;
    }
    source_pos = &(ship.getSpotAt(source_floor_num, source_x, source_y));
    dest_pos = &(ship.getSpotAt(dest_floor_num, dest_x, dest_y));
    if (!source_pos->getAvailable() || source_pos->getContainer() == nullptr ||
        !dest_pos->getAvailable() || dest_pos->getContainer() != nullptr) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id + "- using unavailable spot.");
        return false;
    }
    Container *cont = source_pos->getContainer();
    //Container validation
    if (cont_id != cont->getID()) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id + "- that isn't in the given spot.");
        return false;
    }
    // Balance validation
    if (calc.tryOperation('U', cont->getWeight(), source_x, source_y) != WeightBalanceCalculator::APPROVED
        || calc.tryOperation('L', cont->getWeight(), dest_x, dest_y) != WeightBalanceCalculator::APPROVED) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Move a container with ID: " + cont_id + "- cause the ship to unbalance.");
        return false;
    } else {
        if (source_floor_num != ship.getNumOfDecks() - 1) {
            pos_above = &(ship.getSpotAt(source_floor_num + 1, source_x, source_y));
            if (pos_above->getContainer() != nullptr) { // check if there is a container at the floor above
                Simulator::insertError(num_of_algo, num_of_travel,
                        "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                        "- Move a container with ID: " + cont_id + "- while there's a container above it.");
                return false;
            }
            if (dest_floor_num != 0) {
                pos_below = &(ship.getSpotAt(dest_floor_num - 1, dest_x, dest_y));
                if (pos_below->getAvailable() &&
                    pos_below->getContainer() == nullptr) { // check if there is no container at the floor below
                    Simulator::insertError(num_of_algo, num_of_travel,
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
Simulation::validateRejectOp(Route &travel,
                             int floor_num, int x, int y, const string &cont_id, bool &has_potential_to_be_loaded) {
    Container *cont;
    if (!Container::validateID(cont_id) || ship.isContOnShip(cont_id)) {
        if (travel.getCurrentPort().getNumOfDuplicates(cont_id) > 0) {
            // ID is duplicated
            travel.getCurrentPort().decreaseDuplicateId(cont_id); // one duplicated got detected.
        }
        return true; // Container got rejected cause of bad ID, which is legal!
    }
    cont = travel.getCurrentPort().getWaitingContainerByID(cont_id, false); // get a container from the port
    //Container validation
    if (cont == nullptr && !ship.isContOnShip(cont_id)) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Reject a container with ID: " + cont_id +
                                      "- that wasn't provided by the port.");
        return false; // Given id_cont is not in the waiting list
    }
    if (cont->getSpotInFloor() != nullptr) { // The container was loaded though reported rejected.
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Reject a container with ID: " + cont_id + "- that was already loaded.");
        return false;
    }
    if (travel.getCurrentPort().getNumOfDuplicates(cont_id) > 0) {
        // ID is duplicated
        travel.getCurrentPort().decreaseDuplicateId(cont_id); // one duplicated got detected.
        return true;
    } else if (cont->isValid() && travel.isInRoute(cont->getDestPort()) && this->curr_port_name !=
                                                                           cont->getDestPort()) { // Check if the container's weight and destination are valid.
        if (ship.getNumOfFreeSpots() > 0) {
            Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
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
        if (conts[i].getSpotInFloor() !=
            nullptr) { // find a container that was loaded on the ship, note that the container must be valid.
            max_ind = i; // i is always being raised
        }
    }
    return max_ind;
}

/*
 * Returns the maximal index of container in the vector, that it's destination is the given destination.
 */
int getContainerIndexByDestination(vector<Container> &containers, const string &port_name) {
    int index = 0;
    for (int i = 0; i < (int) containers.size(); ++i) {
        if (containers[i].getDestPort() == port_name)
            index = i;
    }
    return index;
}

/**
 * Checks if the given container has destination that isn't closer than any container that was loaded on the ship from the current port.
 */
bool checkSortedContainers(vector<Container> &conts, Route &travel, const string &cont_id) {
    int farthest_port_num;
    vector<Container> temp_containers = conts;
    travel.sortContainersByDestination(temp_containers);
    farthest_port_num = getFarthestDestOfContainerIndex(
            temp_containers); // get the maximal index of a container that was load to the ship.
    if (getContainerIndexByDestination(temp_containers,
                                       Port::getContainerByIDFrom(temp_containers, cont_id)->getDestPort()) <
        farthest_port_num) {
        return false;
    }
    return true;
}

// Validates all the containers that were left at the port at the end of travel.
void Simulation::checkRemainingContainers(map<string, Container *> &unloaded_containers,
                                          map<string, Container *> &rejected_containers, Port &curr_port) {
    for (const auto &entry : unloaded_containers) {
        if (curr_port.getWaitingContainerByID(entry.first) != nullptr) {
            // In case the container was from the port, note that the container.isValid() is true
            if (rejected_containers.find(entry.first) !=
                rejected_containers.end()) { // check if the container was also rejected. if so, the container had a potential to be loaded on the ship.
                if (!ship.isFull()) {
                    Simulator::insertError(num_of_algo, num_of_travel,
                            "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                            "- Rejected a container with ID: " + entry.second->getID() +
                            "- although it can be loaded correctly.");
                    this->err_in_travel = true;
                } else if (!checkSortedContainers(curr_port.getWaitingContainers(), travel,
                                                  entry.first)) { // check if the container was rejected mistakenly
                    Simulator::insertError(num_of_algo, num_of_travel,
                            "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                            "- Rejected a container with ID: " + entry.second->getID() +
                            "- while another container was loaded and it's destination port is further.");
                    this->err_in_travel = true;
                }
            } // <<< it is not possible to reach the else statement of that if
        } else { // In case the container was from the ship
            if (entry.second->getDestPort() != curr_port.getName()) { // The wrong container got unloaded!
                Simulator::insertError(num_of_algo, num_of_travel,
                        "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                        "- A container with ID: " + entry.second->getID() +
                        "- was left in a port that's different from container's destination.");
                this->err_in_travel = true;
            }
        }
    }
}

void Simulation::checkPortContainers(vector<string> &ignored_containers, Port &curr_port) {
    Container *ignored_cont = nullptr;
    for (auto &container_id : ignored_containers) { // for each container that came from this port that was not treated.
        Simulator::insertError(num_of_algo, num_of_travel,
                "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                "- A container with ID: " + container_id +
                "- was left at the port without getting an instruction.");
        //Check sorted containers
        ignored_cont = curr_port.getWaitingContainerByID(container_id, true); // get valid container from the port
        if (ignored_cont == nullptr) // didn't find valid container
            continue;
        if (travel.isInRoute(ignored_cont->getDestPort()) && this->curr_port_name != ignored_cont->getDestPort() &&
            !checkSortedContainers(curr_port.getWaitingContainers(), travel, container_id)) {
            Simulator::insertError(num_of_algo, num_of_travel,
                    "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                    "- A container with ID: " + container_id +
                    "- was left in port while another container was loaded and it's destination port is further.");
        }
        this->err_in_travel = true;
    }
    for (auto &cont : curr_port.getDuplicateIdOnPort()) { // for each duplicated container that came from this port that was not treated.
        if (cont.second > 0) {
            Simulator::insertError(num_of_algo, num_of_travel,
                    "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                    "- A container with ID: " + cont.first +
                    "- did not get rejected though it has duplicated ID.");
            this->err_in_travel = true;
        }
    }
}

bool
Simulation::validateCargoInstruction(vector<string> &instruction, vector<string> &ignoredContainers,
                                     Container **cont_to_load, Port &current_port,
                                     AbstractAlgorithm::Action &command,
                                     const map<string, Container *> &unloaded_containers) {
    if (!validateInstruction(instruction)) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                                      "- Invalid instruction detected.");
        this->err_in_travel = true;
        return false;
    }
    // Check if the container ID is from the port and delete it.
    auto position = std::find(ignoredContainers.begin(), ignoredContainers.end(), instruction[ContainerID]);
    if (position != ignoredContainers.end()) // if ID was found
        ignoredContainers.erase(position);

    command = actionDic.at(instruction[Command]);
    if (command != AbstractAlgorithm::Action::REJECT) {
        if (!Container::validateID(instruction[ContainerID])) {
            Simulator::insertError(num_of_algo, num_of_travel,
                    "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                    "- Instruction with invalid container ID detected.");
            this->err_in_travel = true;
            return false; // Bad id for container
        }
        *cont_to_load = (unloaded_containers.find(instruction[ContainerID]) != unloaded_containers.end() &&
                         unloaded_containers.at(instruction[ContainerID])->getDestPort() != this->curr_port_name)
                        ? unloaded_containers.at(instruction[ContainerID]) : nullptr;
        if (*cont_to_load ==
            nullptr) {
            *cont_to_load = current_port.getWaitingContainerByID(instruction[ContainerID],
                                                                 false); //Get the container from the port
        }
    }
    return true;
}

void
Simulation::implementInstruction(vector<string> &instruction, AbstractAlgorithm::Action command, int &num_of_operations,
                                 int num_of_algo, Port &current_port, WeightBalanceCalculator &calc,
                                 map<string, Container *> &rejected_containers,
                                 map<string, Container *> &unloaded_containers,
                                 int floor_num, int x, int y, Container *cont_to_load) {
    switch (command) {
        case AbstractAlgorithm::Action::LOAD: {
            if (!validateLoadOp(current_port, calc, floor_num, x, y, cont_to_load)) {
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
            if (!validateUnloadOp(calc, floor_num, x, y, instruction[ContainerID])) {
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
            if (!validateMoveOp(calc, floor_num, x, y, string2int(instruction[DestFloorNum]),
                                string2int(instruction[DestX]), string2int(instruction[DestY]),
                                instruction[ContainerID])) {
                this->err_in_travel = true;
                break;
            }
            // Move container on the ship
            ship.moveContainer(floor_num, x, y, string2int(instruction[DestFloorNum]), string2int(instruction[DestX]),
                               string2int(instruction[DestY]));
            num_of_operations++;
            break;
        }
        case AbstractAlgorithm::Action::REJECT: {
            bool has_potential_to_be_loaded = false;
            if (!validateRejectOp(travel, floor_num, x, y, instruction[ContainerID],
                                  has_potential_to_be_loaded)) {
                this->err_in_travel = true;
                break;
            }
            Container *r_cont = current_port.getWaitingContainerByID(instruction[ContainerID], false);
            rejected_containers.insert({instruction[ContainerID], r_cont});
            if (has_potential_to_be_loaded)
                unloaded_containers.insert({instruction[ContainerID],
                                            r_cont}); // add to unloaded_containers so that we will later check if it was rejected correctly.
            break;
        }
        default: {
            Simulator::insertError(num_of_algo, num_of_travel,
                    "@ Travel: " + this->curr_travel_name + "- Port: " + this->curr_port_name +
                    "- Invalid instruction detected.");
            this->err_in_travel = true;
        }
    }
}

void
Simulation::iterateInstructions(WeightBalanceCalculator &calc, const string &instruction_file, int &num_of_operations,
                                int num_of_algo) {
    FileHandler file(instruction_file);
    vector<string> instruction;
    Container *cont_to_load = nullptr;
    Port &current_port = travel.getCurrentPort();
    map<string, Container *> rejected_containers; // Contains all containers that were rejected correctly.
    map<string, Container *> unloaded_containers; // Contains all containers that were rejected correctly and had potential to be loaded, but ship was full + all containers that were unloaded.
    vector<string> ignored_containers = current_port.getContainersIDFromPort();
    AbstractAlgorithm::Action command;
    while (file.getNextLineAsTokens(instruction)) {
        if (!validateCargoInstruction(instruction, ignored_containers, &cont_to_load, current_port,
                                      command,
                                      unloaded_containers))
            continue;
        implementInstruction(instruction, command, num_of_operations, num_of_algo, current_port, calc,
                             rejected_containers, unloaded_containers,
                             string2int(instruction[FloorNum]), string2int(instruction[X]), string2int(instruction[Y]),
                             cont_to_load);
    }
    checkRemainingContainers(unloaded_containers, rejected_containers, current_port);
    checkPortContainers(ignored_containers, current_port);
}

void Simulation::checkMissedContainers(const string &port_name) {
    if ((int) ship.getContainersForDest(port_name).size() > 0) {
        Simulator::insertError(num_of_algo, num_of_travel,"@ Travel: " + this->curr_travel_name +
                                      "- There are some containers that were not unloaded at their destination port: " +
                                      port_name);
        this->err_in_travel = true;
    }
}

void Simulation::analyzeErrCode(int err_code) {
    vector<unsigned int> one_indexes = getOneIndexes(err_code);
    for (const unsigned int index : one_indexes) {
        if (index > 18) return; // No error code is defined for indexes above 18.
        Simulator::insertError(num_of_algo, num_of_travel,"@ Algorithm reported in travel " + curr_travel_name + ": " + errCodes.at(index));
    }
}