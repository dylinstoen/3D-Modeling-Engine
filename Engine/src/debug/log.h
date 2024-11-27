/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef LOG_H
#define LOG_H

#include <string>
#include <functional>
#include <map>
#include <enum.h>

class Logger
{
public:
    typedef std::function<void(std::string, Priority)> LogListener;

    Logger();

    // Writes a message with given priority
    void WriteLine(std::string message, Priority p = Priority::Normal);

    void AttachListener(std::string name, LogListener listener);
    void DetachListener(std::string name);
private:
    std::map<std::string, LogListener> listeners_;
};

#endif // LOG_H
