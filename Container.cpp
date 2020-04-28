
#include "Container.h"
#include <locale>

set<string> Container::ids;

Container::Container(int _weight, string _dest_port, const string _id) : spot_in_floor(nullptr), id(_id) {
    setWeight(_weight);
    setDestPort(_dest_port);
    ids.insert(_id);
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

bool Container::validateID(const string &new_id, bool newOne) {
    regex form("([A-Z]{3})([JUZ]{1})([0-9]{7})");
    if (regex_match(new_id, form)) {
        if (!newOne) // Not a new container, just need to valid ID format so return true
            return true;
        if (ids.find(new_id) == ids.end()) { // check if id wasn't enter before
            return true;
        }
        if (newOne)
            cout << "WARNING: Container id: " << new_id << " already in use, container ignored" << endl;
        return false;
    }
    if (newOne)
        cout << "WARNING: Illegal container ID was given: " << new_id << endl;
    return false;
}
