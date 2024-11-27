/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <animator.h>
#include <QCheckBox>
#include <widgets/inspectablewidget.h>

class CheckBox : public QCheckBox, public InspectableWidget
{
    Q_OBJECT
public:
    Signal1<bool> Changed;

    CheckBox(bool value = false, QWidget *parent = Q_NULLPTR);
    void SetChecked(bool checked);

public slots:
    void OnToggled(bool checked);
};

#endif // CHECKBOX_H
