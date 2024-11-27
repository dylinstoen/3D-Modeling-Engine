/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <animator.h>
#include <QLineEdit>
#include <widgets/inspectablewidget.h>

class LineEdit : public QLineEdit, public InspectableWidget
{
    Q_OBJECT
public:
    LineEdit(const std::string& value = "", QWidget *parent = Q_NULLPTR);


    // Signals
    Signal1<std::string> Changed;
public slots:
    void OnSetValue(std::string value);
};

#endif // LINEEDIT_H
