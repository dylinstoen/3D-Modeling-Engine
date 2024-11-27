/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <debug/log.h>

Logger::Logger()
{

}

void Logger::WriteLine(std::string message, Priority p) {
    std::cerr << message << std::endl;
    for (auto it = listeners_.begin(); it != listeners_.end(); ++it) {
        it->second(message, p);
    }
}

void Logger::AttachListener(std::string name, LogListener listener) {
    listeners_[name] = listener;
}

void Logger::DetachListener(std::string name) {
    listeners_.erase(name);
}
