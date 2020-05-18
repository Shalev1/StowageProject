#include "BaseAlgorithm.h"

BaseAlgorithm::BaseAlgorithm(){
    // Init the errorCodeBits vector, consider moving to the constructor
    errorCodeBits.push_back(1);
    for(int i = 1; i < NUM_OF_ERROR_CODES; i++){
        errorCodeBits.push_back(errorCodeBits[i-1]*2);
    }
}

int BaseAlgorithm::readShipPlan(const std::string &full_path_and_file_name) {
    ship.resetShipPlan();

    shipValid = true;
    vector<pair<int,string>> err_msgs;
    ship.initShipPlanFromFile(full_path_and_file_name, err_msgs, shipValid);

    //Check for errors
    int errorsFlags = 0;
    for(auto& p : err_msgs){
        errorsFlags |= errorCodeBits[p.first];
    }
    shipErrorCode = 0;
    if(!shipValid)
        shipErrorCode = errorsFlags;
    return errorsFlags;
}

int BaseAlgorithm::readShipRoute(const std::string &full_path_and_file_name) {
    route = Route();
    vector<pair<int,string>> errors;
    routeValid = true;
    route.initRouteFromFile(full_path_and_file_name, errors, routeValid);

    //Check for errors
    int errorsFlags = 0;
    for(auto& p : errors){
        errorsFlags |= errorCodeBits[p.first];
    }
    routeErrorCode = 0;
    if(!routeValid)
        routeErrorCode = errorsFlags;
    return errorsFlags;
}

int BaseAlgorithm::setWeightBalanceCalculator(WeightBalanceCalculator &calculator) {
    weightCal = calculator;
    return 0;
}

int BaseAlgorithm::getInstructionsForCargo(const std::string &input_full_path_and_file_name, const std::string &output_full_path_and_file_name) {
    if(!shipValid){
        FileHandler emptyFile(output_full_path_and_file_name, true); // Create empty instructions file
        return shipErrorCode;
    }
    if(!routeValid){
        FileHandler emptyFile(output_full_path_and_file_name, true); // Create empty instructions file
        return routeErrorCode;
    }
    vector<pair<int,string>> errors;
    route.moveToNextPortWithoutContInit();
    if(!route.hasNextPort() &route.checkLastPortContainers(input_full_path_and_file_name, false)) { // This is the last port and it has waiting containers
        errors.emplace_back(17, "Last port shouldn't has waiting containers");
    } else {
        route.getCurrentPort().initWaitingContainers(input_full_path_and_file_name, errors, ship);
    }
    vector<Container>& waitingContainers = route.getCurrentPort().getWaitingContainers();
    vector<Container*> reloadContainers;
    FileHandler instructionsFile(output_full_path_and_file_name, true);

    // Sort incoming containers by their destination
    route.sortContainersByDestination(waitingContainers);

    // Get Unload instructions for containers with destination equals to this port
    getUnloadInstructions(route.getCurrentPort().getName(), reloadContainers, instructionsFile);

    // Get reload instructions for the reload containers
    getReloadInstructions(reloadContainers, instructionsFile);

    bool fullError = false;
    for (auto & cont : waitingContainers) {
        if(!cont.isValid()){
            // Illegal container, reject
            instructionsFile.writeInstruction("R", cont.getID(), -1, -1, -1);
            continue;
        }
        if (cont.getDestPort() == route.getCurrentPort().getName()) {
            // Destination is the current port, reject
            instructionsFile.writeInstruction("R", cont.getID(), -1, -1, -1);
            continue;
        }
        if (!route.isInRoute(cont.getDestPort())) {
            // Destination is not in the route, reject
            instructionsFile.writeInstruction("R", cont.getID(), -1, -1, -1);
            continue;
        }
        bool notFull = findLoadingSpot(&cont, instructionsFile);
        if(!notFull && !fullError){
            fullError = true;
            errors.emplace_back(18,"Ship is full, rejecting far containers");
        }
    }

    //Check for errors
    int errorsFlags = 0;
    for(auto& p : errors){
        errorsFlags |= errorCodeBits[p.first];
    }
    return errorsFlags;
}

void BaseAlgorithm::getUnloadInstructions(const string &portName, vector<Container *> &reloadContainers,
                                         FileHandler &instructionsFile) {
    Container *container_to_unload;
    // Iterate ship from top to the bottom
    for (int floor_num = ship.getNumOfDecks() - 1; floor_num >= 0; --floor_num) {
        for (int x = 0; x < ship.getShipRows(); ++x) {
            for (int y = 0; y < ship.getShipCols(); ++y) {
                if ((container_to_unload = ship.getContainerAt(floor_num, x, y)) == nullptr) {
                    continue; // empty spot, continue to the next one.
                }
                // Check if the container's port ID match the current port ID
                if (portName == container_to_unload->getDestPort()) {
                    markRemoveContainers(*container_to_unload, *(container_to_unload->getSpotInFloor()),
                                         reloadContainers, instructionsFile);
                }
            }
        }
    }
}

void BaseAlgorithm::getReloadInstructions(vector<Container*>& reload_containers, FileHandler& instructionsFile) {
    for (auto & reload_container : reload_containers) {
        findLoadingSpot(reload_container, instructionsFile);
    }
}

Spot *BaseAlgorithm::getEmptySpot(int &returnFloorNum) {
    for (int floor_num = 0; floor_num < ship.getNumOfDecks(); ++floor_num) {
        //Iterate over the current floor's floor map
        for (int x = 0; x < ship.getShipRows(); ++x) {
            for (int y = 0; y < ship.getShipCols(); ++y) {
                Spot *curSpot = &(ship.getSpotAt(floor_num, x, y));
                // Check if the spot is clear base
                if (curSpot->getAvailable() && curSpot->getContainer() == nullptr) {
                    returnFloorNum = floor_num;
                    return curSpot; //Found an available and empty spot
                }
            }
        }
    }
    return nullptr;
}

bool BaseAlgorithm::findLoadingSpot(Container *cont, FileHandler &instructionsFile) {
    int floorNum;
    Spot *empty_spot = getEmptySpot(floorNum);
    if (empty_spot == nullptr) {
        //Ship is full, reject
        instructionsFile.writeInstruction("R", cont->getID(), -1, -1, -1);
        return false;
    }
    vector<Spot *> failedSpots; // All spots that returned form getEmptySpot but put the container will make the ship unbalance
    // validate that ship will be balance. If not, find another spot.
    while (weightCal.tryOperation('L', cont->getWeight(), empty_spot->getPlaceX(),
                                  empty_spot->getPlaceY()) != WeightBalanceCalculator::APPROVED) {
        empty_spot->setAvailable(false);
        failedSpots.push_back(empty_spot);
        empty_spot = getEmptySpot(floorNum);
        if (empty_spot == nullptr) {
            cout << "WARNING: No available spot for container: " << cont->getID() << endl;
            instructionsFile.writeInstruction("R", cont->getID(), -1, -1, -1);
            for (auto &spot : failedSpots)
                spot->setAvailable(true);
            return true;
        }
    }
    // Spot found, return all failed spots to be available
    for (auto &spot : failedSpots)
        spot->setAvailable(true);
    // Write loading instruction
    instructionsFile.writeInstruction("L", cont->getID(), floorNum, empty_spot->getPlaceX(), empty_spot->getPlaceY());
    ship.insertContainer(empty_spot, *cont);
    return true;
}

bool BaseAlgorithm::checkMoveContainer(Container* cont, Spot& spot, FileHandler& instructionsFile) {
    // Prevent warnings
    (void)cont;
    (void)spot;
    (void)instructionsFile;
    return false; // Naive implementation no move allowed
}

void BaseAlgorithm::markRemoveContainers(Container &cont, Spot &spot, vector<Container *> &reload_containers,
                                        FileHandler &instructionsFile) {
    int curr_floor_num = ship.getNumOfDecks() - 1;
    string curr_dest = cont.getDestPort();
    Spot *curr_spot;
    // Iterate downwards until the specific spot.
    while (curr_floor_num > cont.getSpotInFloor()->getFloorNum()) {
        curr_spot = &(ship.getSpotAt(curr_floor_num, spot.getPlaceX(), spot.getPlaceY()));
        if (curr_spot->getContainer() == nullptr) {
            curr_floor_num--;
            continue;
        }
        if (weightCal.tryOperation('U', cont.getWeight(), curr_spot->getPlaceX(),
                                   curr_spot->getPlaceY()) != WeightBalanceCalculator::APPROVED) { // Check if removing this container will turn the ship out of balance.
            // TODO ex3: Handle error
        }
        if(!checkMoveContainer(curr_spot->getContainer(), *curr_spot, instructionsFile)) {
            reload_containers.push_back(curr_spot->getContainer());
            // Add unload instruction, will be reloaded later
            instructionsFile.writeInstruction("U", curr_spot->getContainer()->getID(), curr_floor_num, spot.getPlaceX(),
                                              spot.getPlaceY());
            ship.removeContainer(curr_spot);
        }
        curr_floor_num--;
    }
    if (weightCal.tryOperation('U', cont.getWeight(), spot.getPlaceX(),
                               spot.getPlaceY()) != WeightBalanceCalculator::APPROVED) { // Check if removing this container will turn the ship out of balance.
        // TODO ex3: Handle error
    }
    // We have reached the container that has the same port ID destination. write unload instruction
    instructionsFile.writeInstruction("U", spot.getContainer()->getID(), curr_floor_num, spot.getPlaceX(),
                                      spot.getPlaceY());
    ship.removeContainer(&spot);
}