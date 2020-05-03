#include "ShipPlan.h"

// Make sure the line is made of only 3 integers
bool validateShipPlanLine(const vector<string> &line, string &err_msg) {
    int i;
    for (i = 0; i < (int) (line.size()); ++i) {
        if (i == 3) {
            err_msg = "Invalid ship fileline was detected- too many arguments in a line.";
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

void ShipPlan::initShipPlanFromFile(const string &file_path, vector<pair<int,string>> &errs_msg, bool &success) {
    FileHandler file(file_path);
    vector<string> line;
    int x, y, unavailable_floors;
    string err;
    if(file.isFailed()){
        errs_msg.emplace_back(3, "Invalid ship plan file was given.");
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
            errs_msg.emplace_back(1, "A spot that exceeds the X/Y ship limits was detected while initializing the ship plan.");
            continue;
        }
        unavailable_floors = this->num_of_decks - string2int(line[2]);
        if (unavailable_floors <= 0) {
            errs_msg.emplace_back(0, "A spot which is available within the maximum number of floors or more was detected while initializing the ship plan.");
            continue;
        } else { //unavailable_floors > 0
            if (!(this->decks[0].getFloorMap()[x][y].getAvailable())) { // TODO: Should I throw an error?
                //spot was already initialized, skip this one
                continue;
            }
            updateSpot(x, y, unavailable_floors);
        }
    }
}

ShipPlan::ShipPlan(const string &file_path, vector<pair<int,string>> &err_msg, bool &success) {
    initShipPlanFromFile(file_path, err_msg, success);
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
    pos->setContainer(&cont);
    cont.setPlace(pos);
    this->free_spots_num--;
}

void ShipPlan::insertContainer(int floor_num, int x, int y, Container &cont) {
    cont.setPlace(&getSpotAt(floor_num, x, y));
    this->getSpotAt(floor_num, x, y).setContainer(&cont);
    this->free_spots_num--;
}

void ShipPlan::removeContainer(Spot *pos) {
    pos->getContainer()->setPlace(nullptr);
    pos->setContainer(nullptr); // clearing spot with null.
    this->free_spots_num++;
}

void ShipPlan::removeContainer(int floor_num, int x, int y) {
    this->getSpotAt(floor_num, x, y).getContainer()->setPlace(nullptr);
    this->getSpotAt(floor_num, x, y).setContainer(nullptr); // clearing spot with null.
    this->free_spots_num++;
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
