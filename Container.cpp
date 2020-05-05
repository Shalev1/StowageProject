
#include "Container.h"
#include <locale>

set<string> Container::simIds;
set<string> Container::algoIds;

Container::Container(int _weight, string _dest_port, const string _id, bool algoCont) :
                     weight(_weight), dest_port(_dest_port), spot_in_floor(nullptr), id(_id) {
    if (algoCont) {
        algoIds.insert(_id);
    } else {
        simIds.insert(_id);
    }
}

string Container::getDestPort() const {
    string temp_dest;
    temp_dest = this->dest_port;
    return temp_dest;
}

void printDestPort(const string dest) {
    cout << "Destination Port: ";
    for (int i = 0; i < (int) (dest.length()); i++) {
        cout << dest[i];
    }
}

ostream &operator<<(ostream &out, const Container &c) {
    out << "Container details are- ";
    printDestPort(c.dest_port);
    return out << " ID: " << c.id << ", "
               << "Weight: " << c.weight
               << endl;
}

bool Container::validateID(const string &id) {
    regex form("([A-Z]{3})([JUZ]{1})([0-9]{7})");
    return regex_match(id, form);

}

bool Container::checkUnique(const string& id, bool algoCheck) {
    if (!algoCheck && simIds.find(id) == simIds.end()) { // check if id wasn't enter before, simulator case
        return true;
    }
    return algoCheck && algoIds.find(id) == algoIds.end(); // check if id wasn't enter before, algorithm case
}
