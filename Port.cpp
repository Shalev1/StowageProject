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

void Port::addContainer(int weight, const string &destPort, const string &id) {
    waitingContainers.push_back(new Container(weight, destPort, id));
}

void Port::initWaitingContainers(const string &path, vector<string>& errVector) {
    FileHandler fh(path);
    if(fh.isFailed())
        errVector.emplace_back("Failed to open " + path + " considered as no containers waiting");
    vector<string> tokens;
    while (fh.getNextLineAsTokens(tokens)) {
        if (tokens.size() != 3) {
            errVector.emplace_back("Line with wrong number of parameters in containers file - line ignored");
            continue;
        }
        string id = tokens[0];
        int weight;
        if (isPositiveNumber(tokens[1])) {
            weight = stoi(tokens[1]);
        } else {
            errVector.emplace_back("Illegal weight given for container: " + tokens[1] + " Container ignored");
            continue;
        }
        string dest = tokens[2];
        if (!Port::validateName(dest)) {
            errVector.emplace_back("Illegal destination given for container: " + dest + " Container ignored");
            continue;
        }

        if (Container::validateID(id)) {
            addContainer(weight, Port::nameToUppercase(dest), id);
        }
    }
}

Container *Port::getWaitingContainerByID(const string &id) {
    return Port::getContainerByIDFrom(waitingContainers, id);
}

Container *Port::getContainerByIDFrom(const vector<Container *> &containers, const string &id) {
    for (auto cont : containers) {
        if (cont->getID() == id) {
            return cont; // found the container
        }
    }
    return nullptr; // didn't find the container
}

ostream &operator<<(ostream &os, const Port &p) {
    os << "Port's name: " << p.name << endl;
    for (Container *const &c : p.waitingContainers) {
        if (c == nullptr)
            continue;
        os << "---" << *c;
    }
    return os;
}
