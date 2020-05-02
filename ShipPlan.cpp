#include "ShipPlan.h"

// Make sure the line is made of only 3 integers
bool validateShipPlanLine(const vector<string> &line) {
    int i;
    for (i = 0; i < (int) (line.size()); ++i) {
        if (i == 3) {
            cout << "ERROR: Invalid file was given- too many arguments in a line. Skipping to the next Route..."
                 << endl;
            return false;
        }
        if (!isPositiveNumber(line[i])) {
            cout << "ERROR: Invalid file was given. Skipping to the next Route..." << endl;
            return false;
        }
    }
    if (i != 3) {
        cout << "ERROR: Invalid file was given- not enough arguments in a line. Skipping to the next Route..." << endl;
        return false;
    }
    return true;
}

void ShipPlan::updateSpot(int x, int y, int unavailable_floors) {
    for (int floor_num = 0; floor_num < unavailable_floors; ++floor_num) {
        this->decks[floor_num].getFloorMap()[x][y]->setAvailable(false);
        this->free_spots_num--;
    }
}

int ShipPlan::initShipPlanFromFile(const string &file_path, bool &valid_ctor) {
    FileHandler file(file_path);
    vector<string> line;
    int x, y, unavailable_floors;
    // Initialize ship dimensions
    if (!file.getNextLineAsTokens(line)) {
        cout << "ERROR: Empty file was given. Skipping to the next Travel..." << endl;
        valid_ctor = false;
        return 1;
    }
    if (!validateShipPlanLine(line)) {
        valid_ctor = false;
        return 1;
    }
    if (!validateShipSize(string2int(line[0]), string2int(line[1]), string2int(line[2]))) {
        cout << "ERROR: Invalid file was given- exceeded ship limits. Skipping to the next Travel..." << endl;
        valid_ctor = false;
        return 1;
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
        if (!validateShipPlanLine(line)) {
            valid_ctor = false;
            return 1;
        }
        x = string2int(line[0]);
        y = string2int(line[1]);
        if (!spotInRange(x, y)) {
            cout << "WARNING: A spot that exceeds the X/Y ship limits was detected." << endl;
            continue;
        }
        unavailable_floors = this->num_of_decks - string2int(line[2]);
        if (unavailable_floors <= 0) {
            cout << "WARNING: A spot which is available within the maximum number of floors or more was detected."
                 << endl;
            continue;
        } else { //unavailable_floors > 0
            if (!(this->decks[0].getFloorMap()[x][y]->getAvailable())) {
                //spot was already initialized, skip this one
                continue;
            }
            updateSpot(x, y, unavailable_floors);
        }
    }
    return 0;
}

ShipPlan::ShipPlan(const string &file_path, bool &valid_ctor) {
    initShipPlanFromFile(file_path, valid_ctor);
}

ShipPlan::~ShipPlan() {
    //Iterate over the ship's space and delete the remaining containers
    for (int floor_num = 0; floor_num < num_of_decks; ++floor_num) {
        //Iterate over the current floor's floor map
        for (int x = 0; x < rows; ++x) {
            for (int y = 0; y < cols; ++y) {
                Spot *curr_spot = this->getSpotAt(floor_num, x, y);
                if (curr_spot->getContainer() != nullptr) {
                    delete curr_spot->getContainer(); // deletes container forever
                    curr_spot->getContainer();
                }
            }
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
    pos->setContainer(&cont);
    cont.setPlace(pos);
    this->free_spots_num--;
}

void ShipPlan::insertContainer(int floor_num, int x, int y, Container &cont) {
    cont.setPlace(getSpotAt(floor_num, x, y));
    this->getSpotAt(floor_num, x, y)->setContainer(&cont);
    this->free_spots_num--;
}

void ShipPlan::removeContainer(Spot *pos) {
    pos->getContainer()->setPlace(nullptr);
    pos->setContainer(nullptr); // clearing spot with null.
    this->free_spots_num++;
}

void ShipPlan::removeContainer(int floor_num, int x, int y) {
    this->getSpotAt(floor_num, x, y)->getContainer()->setPlace(nullptr);
    this->getSpotAt(floor_num, x, y)->setContainer(nullptr); // clearing spot with null.
    this->free_spots_num++;
}

void
ShipPlan::moveContainer(int source_floor_num, int source_x, int source_y, int dest_floor_num, int dest_x, int dest_y) {
    Container *cont = this->getSpotAt(source_floor_num, source_x, source_y)->getContainer();
    this->getSpotAt(source_floor_num, source_x, source_y)->setContainer(nullptr); // clearing old spot.
    cont->setPlace(this->getSpotAt(dest_floor_num, dest_x, dest_y));              // setting container's new spot
    this->getSpotAt(dest_floor_num, dest_x, dest_y)->setContainer(cont);          // setting spot's new container
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
