#include "Port.h"

Port::Port(const string &name) {
    if (!validateName(name)) {
        this->name = INVALID;
    } else {
        string upperName = name;
        for (int i = 0; i < (int) name.length(); i++) {
            upperName[i] = toupper(name[i]);
        }
        this->name = upperName;
    }
}

bool Port::validateName(const string &name) {
    if (name.length() != PORT_NAME_LEN)
        return false;
    for (int i = 0; i < PORT_NAME_LEN; i++) {
        if (!isalpha(name.at(i)))
            return false;
    }
    return true;
}

string Port::nameToUppercase(const string &name) {
    string upperName = name;
    for (int i = 0; i < (int) name.length(); i++) {
        if (islower(name[i])) {
            upperName[i] = toupper(name[i]);
        }
    }
    return upperName;
}

void Port::initWaitingContainers(const string &path, vector<pair<int,string>>& errVector, const ShipPlan& ship) {
    FileHandler fh(path);
    string id = "";
    if (fh.isFailed()){
        errVector.emplace_back(16,"Failed to open " + path + " considered as no containers waiting");
        return;
    }
    vector<string> tokens;
    while (fh.getNextLineAsTokens(tokens)) {
        bool valid = true;
        if (tokens.empty()) {
            errVector.emplace_back(14,"ID cannot be read");
            continue;
        } else {
            id = tokens[0];
            if (!Container::validateID(id)) {
                errVector.emplace_back(15, "Illegal ID for container: " + id);
                valid = false;
            } else {
                if (ship.isContOnShip(id)) { // Check that there isn't already container with the same ID on the ship
                    errVector.emplace_back(11, "Container with ID " + id + " already loaded on the ship");
                    valid = false;
                } else {
                    // Check that there isn't already container with the same ID in the port
                    for (auto &cont: waitingContainers) {
                        if (id == cont.getID()) {
                            errVector.emplace_back(10, "Container with ID: " + id + " already exists in port: " + name);
                            valid = false;
                            break;
                        }
                    }
                }
            }
        }
        int weight = 0;
        if (tokens.size() < 2){
            errVector.emplace_back(12,"No weight given for container: " + id + " - container rejected");
            weight = NO_WEIGHT;
            valid = false;
        } else {
            if (isPositiveNumber(tokens[1])) {
                weight = stoi(tokens[1]);
            } else {
                errVector.emplace_back(12,"Illegal weight given for container: " + tokens[1] + " Container " + id + " rejected");
                weight = ILLEGAL_WEIGHT;
                valid = false;
            }
        }
        string dest;
        if (tokens.size() < 3) {
            errVector.emplace_back(13,"No destination port given for container: " + id + " - container rejected");
            valid = false;
        } else {
            dest = tokens[2];
            if (!Port::validateName(dest)) {
                errVector.emplace_back(13,"Illegal destination given for container: " + dest + " Container " + id + " rejected");
                valid = false;
            }
        }
        waitingContainers.emplace_back(weight, Port::nameToUppercase(dest), id, valid);
    }
}

Container* Port::getWaitingContainerByID(const string &id, bool skipInvalid) {
    return Port::getContainerByIDFrom(waitingContainers, id, skipInvalid);
}

Container* Port::getContainerByIDFrom(vector<Container>& containers, const string &id, bool skipInvalid) {
    for(auto & container : containers){
        if (container.getID() == id) {
            if(skipInvalid && !container.isValid())
                continue;
            return &container; // found the container
        }
    }
    return nullptr; // didn't find the container
}

vector<string> Port::getContainersIDFromPort(){
    vector<string> ids;
    for(auto &cont : waitingContainers){
        ids.push_back(cont.getID());
    }
    return ids;
}

ostream &operator<<(ostream &os, const Port &p) {
    os << "Port's name: " << p.name << endl;
    for (auto& c : p.waitingContainers) {
        os << "---" << c;
    }
    return os;
}

bool Port::isDuplicateOnPort(Container &cont){
    if(cont.isValid())
        return false;
    for(auto& c : waitingContainers){
        if(cont.getID() == c.getID() && c.isValid()){
            return true;
        }
    }
    return false;
}
