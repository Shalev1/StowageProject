#include "BaseAlgorithm.h"

int BaseAlgorithm::readShipPlan(const std::string &full_path_and_file_name) {
    bool valid = true;
    ship.initShipPlanFromFile(full_path_and_file_name, valid);
    return 0;
}

int BaseAlgorithm::readShipRoute(const std::string &full_path_and_file_name) {
    route.initRouteFromFile(full_path_and_file_name);
    return 0;
}

int BaseAlgorithm::setWeightBalanceCalculator(WeightBalanceCalculator &calculator) {
    weightCal = calculator;
    return 0;
}

int BaseAlgorithm::getInstructionsForCargo(const std::string &input_full_path_and_file_name, const std::string &output_full_path_and_file_name) {
    route.moveToNextPortWithoutContInit();
    route.getCurrentPort().initWaitingContainers(input_full_path_and_file_name);
    vector<Container*>& waitingContainers = route.getCurrentPort().getWaitingContainers();

    vector<Container *> reloadContainers;
    FileHandler instructionsFile(output_full_path_and_file_name, true);

    // Get Unload instructions for containers with destination equals to this port
    getUnloadInstructions(route.getCurrentPort().getName(), reloadContainers, instructionsFile);

    // Get reload instructions for the reload containers
    getReloadInstructions(reloadContainers, instructionsFile);

    // Sort incoming containers by their destination
    route.sortContainersByDestination(waitingContainers);

    for (auto it = waitingContainers.begin(); it != waitingContainers.end(); ++it) {
        if ((*it)->getDestPort() == route.getCurrentPort().getName()) {
            instructionsFile.writeInstruction("R", (*it)->getID(), -1, -1, -1); // TODO: ex2 return error code
            cout << "WARNING: Container: " << (*it)->getID()
                 << " will not be loaded, its destination is the current port." << endl;
            continue;
        }
        if (!route.isInRoute((*it)->getDestPort())) {
            instructionsFile.writeInstruction("R", (*it)->getID(), -1, -1, -1); // TODO: ex2 return error code
            cout << "WARNING: Container: " << (*it)->getID()
                 << " will not be loaded, its destination is not a part of the remaining route." << endl;
            continue;
        }
        findLoadingSpot(*it, instructionsFile);
    }
    return 0;
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

void BaseAlgorithm::findLoadingSpot(Container *cont, FileHandler &instructionsFile) {
    int floorNum;
    Spot *empty_spot = getEmptySpot(floorNum);
    if (empty_spot == nullptr) {
        cout << "WARNING: Ship is full, unable to load container: " << cont->getID() << endl;
        instructionsFile.writeInstruction("R", cont->getID(), -1, -1, -1);
        return;
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
            return;
        }
    }
    // Spot found, return all failed spots to be available
    for (auto &spot : failedSpots)
        spot->setAvailable(true);
    // Write loading instruction
    instructionsFile.writeInstruction("L", cont->getID(), floorNum, empty_spot->getPlaceX(), empty_spot->getPlaceY());
    ship.insertContainer(empty_spot, *cont);
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
        reload_containers.push_back(curr_spot->getContainer());
        // Add unload instruction, will be reloaded later TODO: change to move instruction in ex2
        instructionsFile.writeInstruction("U", curr_spot->getContainer()->getID(), curr_floor_num, spot.getPlaceX(),
                                          spot.getPlaceY());
        ship.removeContainer(curr_spot);
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
    delete &cont;
}

