#include "Utils.h"

int string2int(const string &s) {
    return (std::stoi(s));
}

bool isNumber(const string &s) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;
    char *p;
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

void convertFileIntoVector(FileHandler &file, vector<vector<string>> &data) {
    vector<string> line;
    while (file.getNextLineAsTokens(line)) {
        data.push_back(line);
    }
}

bool printCSVFile(const string &file_path) {
    FileHandler file(file_path);
    vector<vector<string>> data;
    if(file.isFailed()){ // File didn't load succesfully
        return false;
    }
    convertFileIntoVector(file, data);
    for (unsigned int i = 0; i < data.size(); ++i) { // for each line
        for (unsigned int j = 0; j < data[i].size(); ++j) {
            if (j == data[i].size() - 1) {
                cout << data[i][j] << '\n';
            } else{
                cout << data[i][j] << ',';
            }
        }
    }
    cout << endl;
    return true;
}

bool dirExists(const string &p){
    return std::filesystem::is_directory(std::filesystem::path(p));
}

string createInstructionDir(const string &output_path, const string &algorithm_name, const string &travel_name){
    string dir_name = output_path + std::filesystem::path::preferred_separator + algorithm_name + "_" + travel_name + "_crane_instructions";
    if(dirExists(dir_name)){ // Check if directory already exists.
        return dir_name;
    }
    if(!std::filesystem::create_directory(dir_name)){
        return ""; // Directory did not open correctly.
    }
    return dir_name;
}

//---FileHandler Functions---//
FileHandler::FileHandler(const string &path, bool truncFlag) : fs(), path(path) {
    if (truncFlag) {
        fs.open(path, std::ios::out | std::ios::trunc);
    } else
        fs.open(path);
    if (!fs.is_open()) {
        fail = true;
    }
}

bool FileHandler::getNextLine(string &line) {
    while (getline(this->fs, line)) {
        line = trimWhitespaces(line);
        if (line[0] == '#')
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
        fs << cell << "\n";
    }
}

void FileHandler::writeInstruction(const string &type, const string &contID, int floor, int x, int y,
                                   int moveFloor, int moveX, int moveY) {
    char sep = ',';
    fs << type << sep << contID << sep << floor << sep << x << sep << y;
    if (type == "M")
        fs << sep << moveFloor << sep << moveX << sep << moveY;
    fs << '\n';
}

void FileHandler::deleteFile(const string &path) {
    remove(path.c_str());
}

FileHandler::~FileHandler() {
    fs.close();
}