#ifndef STOWAGEPROJECT_UTILS_H
#define STOWAGEPROJECT_UTILS_H

#include <iostream>
#include <fstream>
#include <vector>

using std::cout;
using std::string;
using std::fstream;
using std::vector;
using std::endl;

/**
 * Return the given string as tokens in @param tokens using @param delimiter as the delimiter
 */
void getTokens(string s, const string &delimiter, vector<string> &tokens);

/**
 * Check if the given string is  positive number
 */
bool isPositiveNumber(const string &s);

/**
 * Check if the given string is number
 */
bool isNumber(const string & s);

/**
 * Convert string to int
 */
int string2int(const string& s);

bool printCSVFile(const string &file_path);

// Class to deal with files
class FileHandler {
private:
    fstream fs;
    string path;
    bool fail = false; // True if the file wasn't open
public:
    //---Constructors and Destructors---//
    FileHandler(const string &path, bool truncFlag = false); // truncFlag true means to erase the opening file in path
    FileHandler(const FileHandler &other) = delete;
    FileHandler &operator=(const FileHandler &other) = delete;
    ~FileHandler();

    //---Class Functions---//

    bool isFailed(){
        return fail;
    }

    /**
     * return the next line in the file as string
     */
    bool getNextLine(string &line);

    /**
     * return the next line in the file as tokens
     */
    bool getNextLineAsTokens(vector<string> &tokens, const string &delimiter = ",");

    /**
     * Write cell in the result/error table
     */
    void writeCell(const string &cell, bool nextLine = false);

    /**
     * Write instruction in the file (used for the instructions file)
     */
    void writeInstruction(const string& type, const string& contID, int floor, int x, int y,
            int moveFloor = -1, int moveX = -1, int moveY = -1);

    static void deleteFile(const string &path);


};

#endif //STOWAGEPROJECT_UTILS_H
