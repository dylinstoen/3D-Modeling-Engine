/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>
#include <widgets/inspectablewidget.h>

class ComboBox : public QComboBox, public InspectableWidget
{
    Q_OBJECT
public:
    Signal1<int> Changed;

    ComboBox(std::vector<std::string> choices, int selected = 0, QWidget *parent = Q_NULLPTR);
    void SetCurrentIndex(int index);

public slots:
    void OnSelected(int selection);
};

#endif // COMBOBOX_H
