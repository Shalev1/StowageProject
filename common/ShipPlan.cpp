#include "ShipPlan.h"

// Make sure the line is made of only 3 integers
bool validateShipPlanLine(const vector<string> &line, string &err_msg) {
    int i;
    for (i = 0; i < (int) (line.size()); ++i) {
        if (i == 3) {
            err_msg = "Invalid ship file line was detected- too many arguments in a line.";
            return false;
        }
        if (!isPositiveNumber(line[i])) {
            err_msg = "Invalid ship file line was detected.";
            return false;
        }
    }
    if (i != 3) {
        err_msg = "Invalid file was line was detected- not enough arguments in a line.";
        return false;
    }
    return true;
}

void ShipPlan::updateSpot(int x, int y, int unavailable_floors) {
    for (int floor_num = 0; floor_num < unavailable_floors; ++floor_num) {
        this->decks[floor_num].getFloorMap()[x][y].setAvailable(false);
        this->free_spots_num--;
    }
}

int ShipPlan::getUnavailableFloorsNum(int x, int y) {
    int counter = 0;
    while (!decks[counter].getFloorMap()[x][y].getAvailable() && counter < num_of_decks) {
        counter++;
    }
    return counter;
}

void ShipPlan::initShipPlanFromFile(const string &file_path, vector<pair<int, string>> &errs_msg, bool &success) {
    FileHandler file(file_path);
    vector<string> line;
    int x, y, unavailable_floors;
    string err;

    if (file.isFailed()) {
        errs_msg.emplace_back(3, "Failed opening the ship plan file that was given.");
        success = false; // should skip to the next travel
        return;
    }
    // Initialize ship dimensions
    if (!file.getNextLineAsTokens(line)) {
        errs_msg.emplace_back(3, "Empty ship plan file was given.");
        success = false; // should skip to the next travel
        return;
    }
    if (!validateShipPlanLine(line, err)) {
        errs_msg.emplace_back(3, err);
        success = false; // should skip to the next travel
        return;
    }
    if (!validateShipSize(string2int(line[0]), string2int(line[1]), string2int(line[2]))) {
        errs_msg.emplace_back(3, "Invalid ship plan file was given- exceeded ship limits.");
        success = false; // should skip to the next travel
        return;
    }
    setNumOfDecks(string2int(line[0]));
    setShipRows(string2int(line[1]));
    setShipCols(string2int(line[2]));

    // Initialize the floors
    for (int i = 0; i < this->num_of_decks; ++i) {
        this->decks.emplace_back(i, this->rows, this->cols);
    }
    this->free_spots_num = this->rows * this->cols * this->num_of_decks;
    while (file.getNextLineAsTokens(line)) {
        if (!validateShipPlanLine(line, err)) {
            errs_msg.emplace_back(2, err);
            continue;
        }
        x = string2int(line[0]);
        y = string2int(line[1]);
        if (!spotInRange(x, y)) {
            errs_msg.emplace_back(1,
                                  "A spot that exceeds the X/Y ship limits was detected while initializing the ship plan: x = " +
                                  line[0] + "; y = " + line[1] + ";");
            continue;
        }
        unavailable_floors = this->num_of_decks - string2int(line[2]);
        if (unavailable_floors <= 0) {
            errs_msg.emplace_back(0,
                                  "A spot which is available within the maximum number of floors or more was detected while initializing the ship plan: Available floors given is " +
                                  line[2]);
            continue;
        } else { //unavailable_floors > 0
            if (!(this->decks[0].getFloorMap()[x][y].getAvailable())) { // In case the same spot was already initialized
                if (getUnavailableFloorsNum(x, y) == unavailable_floors) {
                    errs_msg.emplace_back(2,
                                          "A spot which was already initialized with the same number of available floors was detected while initializing the ship plan: Spot indexes are x = " +
                                          line[0] + "; y = " + line[1] + ";");
                    continue;
                } else {
                    errs_msg.emplace_back(2,
                                          "A spot which was already initialized with a different number of available floors was detected while initializing the ship plan: Spot indexes are x = " +
                                          line[0] + "; y = " + line[1] + ";");
                    success = false; // should skip to the next travel
                    return;
                }
            }
            updateSpot(x, y, unavailable_floors);
        }
    }
}

ostream &operator<<(ostream &out, const ShipPlan &s) {
    out << "The ship size is: " << s.rows << "," << s.cols
        << "," << s.num_of_decks << "(Rows,Colums,Height)" << endl;
    for (int i = 0; i < s.num_of_decks; ++i) {
        out << s.decks[i];
    }
    return out << endl;
}

bool ShipPlan::spotInRange(int x, int y) const {
    if ((x < 0) || (x >= this->rows) || (y < 0) || (y >= this->cols)) {
        return false;
    }
    return true;
}

bool ShipPlan::validateShipSize(int x, int y, int z) const {
    if (x <= 0 || y <= 0 || z <= 0) {
        return false;
    }
    return true;
}

void ShipPlan::insertContainer(Spot *pos, Container &cont) {
    containers_ids.insert(cont.getID());
    pos->setContainer(&cont);
    cont.setPlace(pos);
    if (containers_to_dest.find(cont.getDestPort()) != containers_to_dest.end())
        containers_to_dest[cont.getDestPort()] = {&cont};
    else
        containers_to_dest[cont.getDestPort()].insert(&cont);
    this->free_spots_num--;
}

void ShipPlan::insertContainer(int floor_num, int x, int y, Container &cont) {
    insertContainer(&getSpotAt(floor_num, x, y), cont);
}

void ShipPlan::removeContainer(Spot *pos) {
    containers_ids.erase(pos->getContainer()->getID());
    containers_to_dest[pos->getContainer()->getDestPort()].erase(pos->getContainer());
    pos->getContainer()->setPlace(nullptr);
    pos->setContainer(nullptr); // clearing spot with null.
    this->free_spots_num++;
}

void ShipPlan::removeContainer(int floor_num, int x, int y) {
    removeContainer(&getSpotAt(floor_num, x, y));
}

void
ShipPlan::moveContainer(int source_floor_num, int source_x, int source_y, int dest_floor_num, int dest_x, int dest_y) {
    Container *cont = this->getSpotAt(source_floor_num, source_x, source_y).getContainer();
    this->getSpotAt(source_floor_num, source_x, source_y).setContainer(nullptr); // clearing old spot.
    cont->setPlace(&(this->getSpotAt(dest_floor_num, dest_x, dest_y)));              // setting container's new spot
    this->getSpotAt(dest_floor_num, dest_x, dest_y).setContainer(cont);          // setting spot's new container
}

vector<Container *> ShipPlan::getContainersForDest(const string &port_name) {
    vector<Container *> containers;
    Container *container_for_port;
    for (int floor_num = 0; floor_num < this->num_of_decks; ++floor_num) {
        for (int x = 0; x < this->rows; ++x) {
            for (int y = 0; y < this->cols; ++y) {
                if ((container_for_port = this->getContainerAt(floor_num, x, y)) == nullptr) {
                    continue; // empty spot, continue to the next one.
                }
                // Check if the container's port ID match the port ID
                if (port_name == container_for_port->getDestPort()) {
                    containers.push_back(container_for_port);
                }
            }
        }
    }
    return containers;
}

bool ShipPlan::isContOnShip(const string &id) const {
    return containers_ids.find(id) != containers_ids.end();
}

void ShipPlan::resetShipPlan() {
    setNumOfDecks(0);
    setShipRows(0);
    setShipCols(0);
    free_spots_num = 0;
    getShipDecks().clear();
    containers_ids.clear();
}

set<Container*>* ShipPlan::getContainersSetForPort(const string &dest) {
    if (containers_to_dest.find(dest) != containers_to_dest.end())
        return &containers_to_dest[dest];
    return nullptr;
}

bool ShipPlan::isUniqueDestInSpot(Spot *spot, int start_floor) {
    if(spot == nullptr || spot->getContainer() == nullptr)
        return false;
    string dest = spot->getContainer()->getDestPort();
    for(int i = start_floor; i < num_of_decks; i++) {
        if (getContainerAt(i, spot->getPlaceX(), spot->getPlaceY()) == nullptr)
            continue;
        if (getContainerAt(i, spot->getPlaceX(), spot->getPlaceY())->getDestPort() != dest)
            return false;
    }
    return true;
}

bool ShipPlan::isUniqueDestAboveSpot(Spot *spot) {
    return isUniqueDestInSpot(spot, spot->getFloorNum());
}

Spot* ShipPlan::getFirstFreeSpotIn(int x, int y) {
    for(int floor_num = 0; floor_num < num_of_decks; floor_num++) {
        Spot& spot = getSpotAt(floor_num, x, y);
        if(spot.getAvailable() && spot.getContainer() == nullptr)
            return &spot;
    }
    return nullptr;
}

Spot* ShipPlan::getFirstAvailableSpotIn(int x, int y) {
    for(int floor_num = 0; floor_num < num_of_decks; floor_num++) {
        Spot& spot = getSpotAt(floor_num, x, y);
        if(spot.getAvailable())
            return &spot;
    }
    return nullptr;
}

string ShipPlan::getClosestDestInSpot(int x, int y, const Route &route) {
    string closest_dest;
    int closet_dist = route.getNumOfPorts() + 1;
    for(int floor_num = 0; floor_num < num_of_decks; floor_num++) {
        Spot& spot = getSpotAt(floor_num, x, y);
        if(spot.getAvailable() && spot.getContainer() != nullptr){
            int dist = route.stopsUntilPort(spot.getContainer()->getDestPort());
            if(dist < closet_dist){
                closest_dest = spot.getContainer()->getDestPort();
                closet_dist = dist;
            }
        }
    }

    return closest_dest;
}