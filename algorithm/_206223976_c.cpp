//
// Created by tomer on 17/05/2020.
//

#include "_206223976_c.h"
REGISTER_ALGORITHM(_206223976_c)

void _206223976_c::markRemoveContainers(Container &cont, Spot &spot, vector<Container *> &reload_containers,
                                        FileHandler &instructionsFile) {
    if(spot.getFloorNum() == 0 && spot.getPlaceX() == 0 && spot.getPlaceY() == 0)
        return;
    BaseAlgorithm::markRemoveContainers(cont, spot, reload_containers, instructionsFile);
}

int _206223976_c::getInstructionsForCargo(const std::string &input_full_path_and_file_name, const std::string &output_full_path_and_file_name) {
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
        route.getCurrentPort().initWaitingContainers(input_full_path_and_file_name, errors, ship, route.getLeftPortsNames());
    }
    vector<Container>& waitingContainers = route.getCurrentPort().getWaitingContainers();
    vector<Container*> reloadContainers;
    FileHandler instructionsFile(output_full_path_and_file_name, true);

    if(!route.hasNextPort()){
        int floorNum;
        Spot* s = getEmptySpot(floorNum);
        if(s != nullptr)
            instructionsFile.writeInstruction("L", "DDAU9915525", floorNum, s->getPlaceX(), s->getPlaceY());
    }


    // Sort incoming containers by their destination
    route.sortContainersByDestination(waitingContainers);

    // Get Unload instructions for containers with destination equals to this port
    getUnloadInstructions(route.getCurrentPort().getName(), reloadContainers, instructionsFile);

    if(!reloadContainers.empty())
        reloadContainers.erase(reloadContainers.begin());
    // Get reload instructions for the reload containers
    getReloadInstructions(reloadContainers, instructionsFile);

    bool fullError = false;
    bool firstCont = true;
    for (auto & cont : waitingContainers) {
        if(ship.getNumOfFreeSpots() == 1)
            break;
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
        // Reject duplicate containers
        for(int i = 0; i < route.getCurrentPort().getNumOfDuplicates(cont.getID()); i++) {
            int floorNum;
            Spot *s = getEmptySpot(floorNum);
            if (s != nullptr) {
                ship.insertContainer(s, *route.getCurrentPort().getWaitingContainerByID(cont.getID()));
                instructionsFile.writeInstruction("L", cont.getID(), floorNum, s->getPlaceX(), s->getPlaceY());
            }
        }
        if(firstCont){
            firstCont = false;
            instructionsFile.writeInstruction("R", cont.getID(), -1, -1, -1);
            continue;
        }
        bool notFull = findLoadingSpot(&cont, instructionsFile);
        if(!notFull && !fullError){
            fullError = true;
            errors.emplace_back(18,"Ship is full, rejecting far containers");
        }
    }

    if(ship.getNumOfFreeSpots() == 1) {
        int floorNum;
        Spot *s = getEmptySpot(floorNum);
        if (!waitingContainers.empty()) {
            Container &lastCont = waitingContainers[(int) waitingContainers.size() - 1];
            if (s != nullptr) {
                ship.insertContainer(s, lastCont);
                instructionsFile.writeInstruction("L", lastCont.getID(), floorNum, s->getPlaceX(), s->getPlaceY());
            }
        }
    }

    //Check for errors
    int errorsFlags = 0;
    for(auto& p : errors){
        errorsFlags |= errorCodeBits[p.first];
    }
    return errorsFlags;
}