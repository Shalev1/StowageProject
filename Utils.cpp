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

void fillWhiteSpaces(int word_len, int max_len) {
    for (int i = 0; i < max_len - word_len; ++i) {
        cout << " ";
    }
}

void printSeperators(int max_word_len, int max_num_of_words) {
    cout << "__";
    for (int i = 0; i < (max_word_len * max_num_of_words) + (max_num_of_words * 2); ++i) {
        cout << "_";
    }
    cout << endl;
}

void getLineStat(vector<vector<string>> &data, int &max_line_len, int &max_num_of_words, int &max_word_len) {
    int char_counter;
    int len;
    for (unsigned int i = 0; i < data.size(); ++i) { // for each line
        char_counter = 0;
        for (unsigned int j = 0; j < data[i].size(); ++j) { // for each word in line
            len = (int)data[i][j].length();
            char_counter += len;
            if (max_word_len < len) // Finds the longest word
                max_word_len = len;
        }
        if(max_num_of_words < (int)data[i].size()) // Finds the highest word number in lines
            max_num_of_words = (int)data[i].size();
        if(max_line_len < char_counter) // Finds the longest line
            max_line_len = char_counter;
    }
}

bool printCSVFile(const string &file_path) {
    FileHandler file(file_path); // TODO: need to make sure file is opened correctly
    vector<vector<string>> data;
    convertFileIntoVector(file, data);
    int max_line_len = 0, max_num_of_words = 0, max_word_len = 0;
    getLineStat(data, max_line_len, max_num_of_words, max_word_len);
    printSeperators(max_word_len, max_num_of_words);
    for (unsigned int i = 0; i < data.size(); ++i) { // for each line
        cout << "||";
        for (int j = 0; j < max_num_of_words; ++j) {
            if (j < (int)data[i].size()) {
                cout << data[i][j];
                fillWhiteSpaces((int) data[i][j].length(), max_word_len);
            } else { // There are no more words in line, print whitespaces
                fillWhiteSpaces(0, max_word_len);
            }
            cout << "||";
        }
        cout << endl;
        printSeperators(max_word_len, max_num_of_words);
    }
    return true;
}

bool dirExists(const string &p){
    return std::filesystem::is_directory(std::filesystem::path(p));
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