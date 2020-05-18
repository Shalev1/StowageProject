
#include "Container.h"
#include <locale>

Container::Container(int _weight, string _dest_port, const string _id, bool valid) :
                     weight(_weight), dest_port(_dest_port), spot_in_floor(nullptr), id(_id), valid(valid) {
}

string Container::getDestPort() const {
    string temp_dest;
    temp_dest = this->dest_port;
    return temp_dest;
}

void printDestPort(const string& dest) {
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
    return ISO_6346::isValidId(id);

}