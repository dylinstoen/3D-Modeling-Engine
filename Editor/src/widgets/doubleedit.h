/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef QDOUBLEEDIT_H
#define QDOUBLEEDIT_H

#include <QDoubleSpinBox>
#include <widgets/inspectablewidget.h>
#include <animator.h>

class DoubleEdit : public QDoubleSpinBox, public InspectableWidget
{
    Q_OBJECT
public:
    Signal1<double> Changed;

    DoubleEdit(double value = 0.0, int precision = 5, QWidget *parent = Q_NULLPTR);

public slots:
    void OnChanged(double value);
    void OnSetValue(double value);
protected:
    virtual QString textFromValue(double val) const;
    int precision_;
    double value_;
};

#endif // QDOUBLEEDIT_H
