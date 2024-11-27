/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QAction>
#include <unordered_map>

class ActionManager {
public:
    ActionManager(QObject *parent = nullptr);

    // By default the key is the name. It's there if there are two actions with the same name.
    QAction* CreateAction(const std::string& name, std::string key = "");
    QAction*& operator[] (const std::string& key);
private:
    QObject* parent_;
    std::unordered_map<std::string, QAction*> actions_;
};

#endif // ACTIONMANAGER_H
