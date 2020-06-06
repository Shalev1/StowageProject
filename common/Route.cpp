#include "Route.h"

Route::Route(const string &path, vector<pair<int,string>>& errVector, bool& success) : currentPortNum(-1) {
    initRouteFromFile(path, errVector, success);
}

void Route::initRouteFromFile(const string& path, vector<pair<int,string>>& errVector, bool& success) {
    FileHandler fh(path);
    if(fh.isFailed()){
        errVector.emplace_back(7,"Error while opening route file- can't run this travel");
        success = false;
    }
    string name;
    string prevName;
    while(fh.getNextLine(name)){
        if(name == prevName){
            errVector.emplace_back(5,"Port " + name + " Appeared twice in a row - second time ignored");
        } else {
            if(Port::validateName(name)) {
                ports.emplace_back(name);
                portVisits[name] = 0;
            } else {
                errVector.emplace_back(6,"Illegal name for port: " + name + " port ignored");
            }
        }
        prevName = name;
    }
    if((int)ports.size() < 1){
        errVector.emplace_back(8,"Illegal Route file given - empty file - can't run this travel");
        success = false;
    } else if((int)ports.size() < 2){
        errVector.emplace_back(8,"Illegal Route file given - less then two valid ports in route - can't run this travel");
        success = false;
    }
    empty_file = string(".") + std::filesystem::path::preferred_separator + string("empty_file");
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

void Route::initPorts(const string &dir, vector<string> &paths, vector<pair<int, string> > &errVector, const ShipPlan& ship) {
    initPortsContainersFiles(dir, paths, errVector);
    map<string, int> portToFileNum;
    int portNumInRoute = -1;
    for(auto& port : ports){
        portNumInRoute++;
        if(portToFileNum.count(port.getName()))
            portToFileNum[port.getName()]++;
        else
            portToFileNum[port.getName()] = 1;
        bool findFile = false;
        // Search for the containers file for the current port and current visit number
        for (auto it = portsContainersPaths.begin(); it != portsContainersPaths.end(); ++it) {
            string portCode = (*it).substr(0, indexOfFirst_InPath);
            if(portCode == port.getName()) {
                string portNumS = (*it).substr(indexOfFirst_InPath + 1, (*it).find('.') - (indexOfFirst_InPath + 1));
                int portNum = stoi(portNumS);
                if (portNum == portToFileNum[port.getName()]) {
                    if (portNumInRoute == (int) ports.size() - 1) { // last port
                        if(checkLastPortContainers(*it, true)){
                            errVector.emplace_back(17,"Last port shouldn't has waiting containers");
                        }
                        portsContainersPathsSorted.push_back(dir + std::filesystem::path::preferred_separator + (*it));
                    } else {
                        string currentPortPath = dir + std::filesystem::path::preferred_separator + (*it);
                        portsContainersPathsSorted.push_back(currentPortPath);
                        ports[portNumInRoute].initWaitingContainers(currentPortPath, errVector, ship, getLeftPortsNames(portNumInRoute));
                    }
                    portsContainersPaths.erase(it);
                    findFile = true;
                    break;
                }
            }
        }
        if(findFile)
            continue;
        portsContainersPathsSorted.push_back(empty_file);
        if(portNumInRoute != (int)ports.size() - 1){
            errVector.emplace_back(-1,"No waiting containers in Port " + port.getName() +
                                      " for visit number: " + to_string(portToFileNum[port.getName()]));
        }
    }
}

void Route::initPortsContainersFiles(const string& dir, vector<string>& paths, vector<pair<int,string>>& errVector){
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
        regex r("[0-9]+\\.cargo_data");
        if(restString[0] == '0' || !regex_match(restString, r)){
            paths.erase(it);
            continue;
        } else {
            if(!portAppearances.count(portCode)){ // Port isn't in the route
                errVector.emplace_back(-1,"File: " + *it + " ignored - port " + portCode + " is not in the route");
                paths.erase(it);
                continue;
            }
            string portNumS = restString.substr(0, restString.find('.'));
            int portNum = stoi(portNumS);
            if(portNum > portAppearances[portCode]){
                errVector.emplace_back(-1,"File: " + *it + " ignored - port " + (portCode += " won't be visited ") + (portNumS += " times"));
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

bool Route::moveToNextPortWithoutContInit() { // TODO: delete this function
    if(!hasNextPort())
        return false;
    currentPortNum++;
    portVisits[getCurrentPort().getName()]++;
    return true;
}

bool Route::checkLastPortContainers(const string& lastPortPath, bool addDir) {
    string path = addDir ? dir + std::filesystem::path::preferred_separator : "";
    path.append(lastPortPath);
    FileHandler lastPortFile(path);
    vector<string> tokens;
    return lastPortFile.getNextLineAsTokens(tokens);
}

bool Route::moveToNextPort(const ShipPlan& ship) {
    if(!hasNextPort())
        return false;
    currentPortNum++;
    portVisits[getCurrentPort().getName()]++;
    for(auto& cont : getCurrentPort().getWaitingContainers()){
        if(ship.isContOnShip(cont.getID())){
            cont.invalidateContainer();
        }
    }
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

void Route::sortContainersByDestination(vector<Container>& containers){
    sort(containers.begin(), containers.end(),[this](Container& c1, Container& c2){
        string dest1 = c1.getDestPort();
        string dest2 = c2.getDestPort();
        if(dest1 == dest2) // same destination, dont care about order for this case
            return false;
        string closeDest = this->getCloserDestination(dest1, dest2);
        return dest1 == closeDest;
    });
}

void Route::sortContainersByFurtherDestination(vector<Container*> &containers) {
    sort(containers.begin(), containers.end(),[this](Container* c1, Container* c2){
        string dest1 = c1->getDestPort();
        string dest2 = c2->getDestPort();
        if(dest1 == dest2) // same destination, dont care about order for this case
            return false;
        string closeDest = this->getCloserDestination(dest1, dest2);
        return dest2 == closeDest;
    });
}

ostream& operator<<(ostream& os, const Route& r){
    os << "The route include the following ports: " << endl;
    for(const Port& p : r.ports){
        os << p;
    }
    return os;
}

vector<string> Route::getLeftPortsNames(int fromPortNum) {
    vector<string> names;
    if(fromPortNum == -1)
        fromPortNum = currentPortNum;
    for(auto it = ports.begin() + fromPortNum; it != ports.end(); ++it){
        names.emplace_back((*it).getName());
    }
    return names;
}

int Route::stopsUntilPort(const string &dest) const{
    int stops = 0;
    for (auto it = ports.begin() + currentPortNum; it != ports.end(); ++it) {
        if(dest == (*it).getName()){
            return stops;
        }
        stops++;
    }
    return -1;
}

Port& Route::getCurrentPort() {
    return ports[currentPortNum];
}

const string & Route::getCurrentPortName() const {
    return ports[currentPortNum].getName();
}

const string & Route::getLastPortName() const {
    return ports[(int)ports.size() - 1].getName();
}

int Route::getNumOfPorts() const {
    return (int)ports.size();
}
