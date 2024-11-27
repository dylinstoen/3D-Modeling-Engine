/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef GLOBAL_H
#define GLOBAL_H

#define _USE_MATH_DEFINES
#include <assert.h>
#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <stack>
#include <vector>
#include <map>
#include <unordered_map>
#include <array>
#include <functional>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <iomanip>
#include <chrono>
#include <signals/Signal.h>

// For signal.h library
using namespace Gallant;

// Non-third party includes
#include <forward.h>
#include <enum.h>
#include <util.h>
#include <debug/log.h>

const unsigned short VF_POS = 1 << 0;
const unsigned short VF_COL = 1 << 1;
const unsigned short VF_TEX = 1 << 2;
const unsigned short VF_NRM = 1 << 3;
const unsigned short VF_BNM = 1 << 4;
const unsigned short VF_TAN = 1 << 5;

// Execptions
struct FileIOException : public std::exception {
    std::string message;
    FileIOException(std::string message) : message(message) {}
    ~FileIOException() throw () {}
    const char* what() const throw() { return message.c_str(); }
};

struct ShaderCompileException : public std::exception {
    std::string message;
    ShaderCompileException(std::string message) : message(message) {}
    ~ShaderCompileException() throw () {}
    const char* what() const throw() { return message.c_str(); }
};

struct ProgramLinkException : public std::exception {
    std::string message;
    ProgramLinkException(std::string message) : message(message) {}
    ~ProgramLinkException() throw () {}
    const char* what() const throw() { return message.c_str(); }
};

struct RenderingException : public std::exception {
    std::string message;
    RenderingException(std::string message) : message(message) {}
    ~RenderingException() throw () {}
    const char* what() const throw() { return message.c_str(); }
};

// Exposes Debug class globally
class Debug
{
public:
    Debug();

    static Logger Log;
private:
};

#endif // GLOBAL_H
