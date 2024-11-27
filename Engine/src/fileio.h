/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef FILEIO_H
#define FILEIO_H

#include <animator.h>
#include <vectors.h>

// Basic disk IO operations
class FileIO
{
public:
    // Reads the contents of the specified textfile into a string.
    // Throws a FileIOException if an error occurred.
    static std::string ReadTextFile(const std::string& filename) {
        std::ostringstream contents;
        std::ifstream file(filename.c_str(), std::ifstream::in);

        // Check if the file was actually opened
        if(!file.is_open()) throw FileIOException("Cannot open file \"" + filename + "\": " + strerror(errno));

        // Read the file line by line into the string stream
        std::string line;
        while(std::getline(file, line)) {
            contents << line << std::endl;
        }

        // Check if Badbit is set
        if(file.bad()) throw FileIOException("Error occurred while reading file \"" + filename + "\": " + strerror(errno));

        file.close();
        return contents.str();
    }

    // Writes the contents of the specified string into a file (overwrite).
    // Throws a FileIOException if an error occurred.
    static void WriteTextFile(const std::string& filename, const std::string& text) {
        std::ofstream file(filename.c_str(), std::ofstream::out);

        // Check if the file was actually opened
        if(!file.is_open()) throw FileIOException("Cannot open file \"" + filename + "\": " + strerror(errno));

        file << text << std::endl;

        // Check if Badbit is set
        if(file.bad()) throw FileIOException("Error occurred while writing to file \"" + filename + "\": " + strerror(errno));

        file.close();
    }

    // Reads the Curve file which comprises on each line a pair of decimal numbers X, Y delimited by a whitespace character
    // It may or may not contain lines with a single '-' character indicating that the Curve file contains another version
    // of the curve with different number of points.
    // Returns a list of list of points. Guaranteed to contain at least one list of points (even though it may be empty).
    // Example:
    //      -0.01 1.5296
    //      0.05936 1.548994375
    //      0.14788 1.576855
    //      -
    //      -0.01 1.5296
    //      0.14788 1.576855
    //      ...
    // Throws a FileIOException if an error occurred.
    static std::vector<std::vector<glm::vec2>> ReadCurveFile(const std::string& filename) {
        std::vector<std::vector<glm::vec2>> points;
        points.emplace_back(std::vector<glm::vec2>());
        int current_list = 0;
        std::ifstream file(filename.c_str(), std::ifstream::in);

        // Check if the file was actually opened
        if(!file.is_open()) throw FileIOException("Cannot open file \"" + filename + "\": " + strerror(errno));

        // Read the file line by line
        std::string line;
        while(std::getline(file, line)) {
            if (line == "-") {
                points.emplace_back(std::vector<glm::vec2>());
                current_list++;
            } else {
                size_t index = line.find(' ');
                std::string x_str = line.substr(0, index);
                std::string y_str = line.substr(index + 1, line.size());
                points.at(current_list).emplace_back(glm::vec2(std::stof(x_str), std::stof(y_str)));
            }
        }

        // Check if Badbit is set
        if(file.bad()) throw FileIOException("Error occurred while reading file \"" + filename + "\": " + strerror(errno));

        file.close();
        return points;
    }
};

#endif // FILEIO_H
