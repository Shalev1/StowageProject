#include "Utils.h"

int string2int(const string& s) {
    const char *c_arr = s.c_str();
    return (atoi(c_arr));
}

bool isNumber(const string & s){
   if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;
   char * p;
   strtol(s.c_str(), &p, 10); // will update p to the first character that is not numeric
   return (*p == 0);
}

string trimWhitespaces(const string &str, const string &whitespace = " \t\v\f\r") {
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == string::npos)
        return ""; // no content
    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}

void getTokens(string s, const string &delimiter, vector<string> &tokens) {
    while (true) {
        int delimiterIndex = s.find(delimiter);
        if (delimiterIndex == -1) {
            tokens.push_back(trimWhitespaces(s));
            break;
        }
        tokens.push_back(trimWhitespaces(s.substr(0, delimiterIndex)));
        s.erase(0, delimiterIndex + delimiter.length());
    }
}

bool isPositiveNumber(const string &s) {
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) {
        ++it;
    }
    return (!s.empty()) && (it == s.end());
}

FileHandler::FileHandler(const string& path, bool truncFlag): fs(), path(path){
    if(truncFlag){
        fs.open(path, std::ios::out | std::ios::trunc);
	}
    else
        fs.open(path);
    if(!fs.is_open()){
        fail = true;
    }
}

bool FileHandler::getNextLine(string &line) {
    while(getline(this->fs,line)){
        line = trimWhitespaces(line);
        if(line[0] == '#')
            continue;
        return true;
    }
    return false;
}

bool FileHandler::getNextLineAsTokens(vector<string> &tokens, const string &delimiter) {
    tokens.clear();
    string tmp;
    while (getNextLine(tmp)) {
        if (tmp[0] == '#')
            continue;
        getTokens(tmp, delimiter, tokens);
        return true;
    }
    return false;
}

void FileHandler::writeCell(const string &cell, bool nextLine) {
    if (!nextLine) {
        fs << cell << ",";
    } else {
        fs << "\n";
    }
}

void FileHandler::writeInstruction(const string& type, const string& contID, int floor, int x, int y,
                                   int moveFloor, int moveX, int moveY) {
    char sep = ',';
    fs << type << sep << contID << sep << floor << sep << x << sep << y;
    if(type == "M")
        fs << sep << moveFloor << sep << moveX << sep << moveY;
    fs << '\n';
}

void FileHandler::deleteFile(const string &path) {
    remove(path.c_str());
}

FileHandler::~FileHandler() {
    fs.close();
}