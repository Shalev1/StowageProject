#include "Route.h"

Route::Route(const string &path) : currentPortNum(-1) {
    initRouteFromFile(path);
}

void Route::initRouteFromFile(const string& path) {
    FileHandler fh(path);
    string name;
    string prevName;
    while(fh.getNextLine(name)){
        if(name == prevName){
            cout << "WARNING: Port " << name << " Appeared twice in a row, second time ignored" << endl;
        } else {
            if(Port::validateName(name)) {
                ports.emplace_back(name);
                portVisits[name] = 0;
            } else {
                cout << "WARNING: Illegal name for port: " << name << " port ignored" << endl;
            }
        }
        prevName = name;
    }
    empty_file = string("Files") + std::filesystem::path::preferred_separator + string("empty_file.csv");
}

/**
 * Compare two port's path for the sort in initPortsContainer
 * First compare the port code and sort alphabetically, second compare the number part, sort from small to big
 */
bool portPathsCompare(const string& s1, const string& s2) {
    string portCode1 = s1.substr(0,indexOfFirst_InPath);
    string portCode2 = s2.substr(0,indexOfFirst_InPath);
    if(portCode1 != portCode2){
        return portCode1 < portCode2;
    } else {
        string num1 = s1.substr(indexOfFirst_InPath + 1, s1.find('.') - (indexOfFirst_InPath + 1));
        string num2 = s2.substr(indexOfFirst_InPath + 1, s2.find('.') - (indexOfFirst_InPath + 1));
        return stoi(num1) < stoi(num2);
    }
}

void Route::initPortsContainersFiles(const string& dir, vector<string>& paths){
    map<string, int> portAppearances; // Map to count how many times each port will be visited
    for(const Port& p : ports){
        if(portAppearances.count(p.getName()))
            portAppearances[p.getName()]++;
        else
            portAppearances[p.getName()] = 1;
    }
    for (auto it = paths.begin(); it != paths.end();) {
        string portCode = (*it).substr(0, indexOfFirst_InPath);
        if(!Port::validateName(portCode) || (*it)[indexOfFirst_InPath] != '_'){
            paths.erase(it);
            continue;
        }
        string restString = (*it).substr(indexOfFirst_InPath + 1);
        regex r("[0-9]+\\.cargo_data.csv");
        if(!regex_match(restString, r)){
            paths.erase(it);
            continue;
        } else {
            if(!portAppearances.count(portCode)){ // Port isn't in the route
                cout << "WARNING: File " << *it <<" ignored, port " << portCode << " is not in the route " << endl;
                paths.erase(it);
                continue;
            }
            string portNumS = restString.substr(0, restString.find('.'));
            int portNum = stoi(portNumS);
            if(portNum > portAppearances[portCode]){
                cout << "WARNING: File " << *it <<" ignored, port " << portCode << " won't be visited "
                    << portNumS << " times" << endl;
                paths.erase(it);
                continue;
            }
            it++;
        }
    }
    sort(paths.begin(), paths.end(), portPathsCompare);
    this->dir = dir;
    this->portsContainersPaths = paths;
}

bool Route::moveToNextPortWithoutContInit() {
    if(!hasNextPort())
        return false;
    currentPortNum++;
    portVisits[getCurrentPort().getName()]++;
    return true;
}

bool Route::moveToNextPort() {
    if(!hasNextPort())
        return false;
    currentPortNum++;
    cout << "-----------------Entering Port: " << getCurrentPort().getName()  << " --------------------" << endl;
    portVisits[getCurrentPort().getName()]++;
    // Search for the containers file for the current port and current visit number
    for (auto it = portsContainersPaths.begin(); it != portsContainersPaths.end(); ++it) {
        string portCode = (*it).substr(0, indexOfFirst_InPath);
        if(portCode == ports[currentPortNum].getName()) {
            string portNumS = (*it).substr(indexOfFirst_InPath + 1, (*it).find('.') - (indexOfFirst_InPath + 1));
            int portNum = stoi(portNumS);
            if (portNum == portVisits[getCurrentPort().getName()]) {
                if (currentPortNum == (int) ports.size() - 1) { // last port
                    cout << "WARNING: Last port shouldn't has a waiting containers file, file ignored" << endl;
                    currentPortPath = empty_file;
                } else {
                    currentPortPath = dir + std::filesystem::path::preferred_separator + (*it);
                    ports[currentPortNum].initWaitingContainers(currentPortPath);
                }
                portsContainersPaths.erase(it);
                return true;
            }
        }
    }
    if(currentPortNum != (int)ports.size() - 1){ // this isn't the last port
        cout << "WARNING: No waiting containers in Port " << getCurrentPort().getName() <<
            " for visit number: " << portVisits[getCurrentPort().getName()]<< endl;
    }
    currentPortPath = empty_file;
    return true;
}

string Route::getCloserDestination(const string &d1, const string &d2) {
    for (auto it = ports.begin() + currentPortNum; it != ports.end(); ++it) {
        if((*it).getName() == d1)
            return d1;
        if((*it).getName() == d2)
            return d2;
    }
    return "Not Found";
}

bool Route::isInRoute(const string &portName) const {
    for(auto it = ports.begin() + currentPortNum; it != ports.end(); ++it)
        if(portName == (*it).getName())
            return true;
    return false;
}

void Route::sortContainersByDestination(vector<Container*>& containers){
    sort(containers.begin(), containers.end(),[this](Container* c1, Container* c2){
        string dest1 = c1->getDestPort();
        string dest2 = c2->getDestPort();
        string closeDest = this->getCloserDestination(dest1, dest2);
        return dest1 == closeDest;
    });
}

void Route::clearCurrentPort() {
    vector<Container*> containers = getCurrentPort().getWaitingContainers();
    for (int i = 0; i < (int)containers.size(); i++) {
        if(containers[i]->getSpotInFloor() == nullptr){
            delete containers[i];
        }
    }
}

ostream& operator<<(ostream& os, const Route& r){
    os << "The route include the following ports: " << endl;
    for(const Port& p : r.ports){
        os << p;
    }
    return os;
}

