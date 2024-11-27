/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef INTEDIT_H
#define INTEDIT_H

#include <QLineEdit>
#include <QIntValidator>
#include <widgets/inspectablewidget.h>
#include <animator.h>

class IntEdit : public QLineEdit, public InspectableWidget
{
    Q_OBJECT
public:
    IntEdit(int value = 0, bool unsigned_only = false, QWidget *parent = Q_NULLPTR);


    // Signals
    Signal1<int> Changed;    
public slots:
    void OnChanged(const QString &text);
    void OnSetValue(int value);
protected:
    static QIntValidator validator_;
    int value_;
    bool unsigned_only_;
};

#endif // INTEDIT_H
