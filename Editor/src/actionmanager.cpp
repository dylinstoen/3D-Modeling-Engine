/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "actionmanager.h"
#include <stdexcept>

ActionManager::ActionManager(QObject *parent) :
    parent_(parent)
{

}

QAction *ActionManager::CreateAction(const std::string& name, std::string key) {
    if (key.empty()) key = name;
    if (actions_.count(key)) throw std::invalid_argument("Action already exists");
    actions_[key] = new QAction(QObject::tr(name.c_str()), parent_);
    return actions_[key];
}

QAction *&ActionManager::operator[](const std::string& key) {
    if (!actions_.count(key)) throw std::invalid_argument("Action does not exist");
    return actions_[key];
}
