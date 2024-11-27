/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SLIDER_H
#define SLIDER_H

#include <QWidget>
#include <widgets/doubleedit.h>
#include <QSlider>
#include <widgets/inspectablewidget.h>

class Slider : public QWidget, public InspectableWidget
{
    Q_OBJECT
public:
    Slider(double min, double max, double value = 0.0, double step = 0.1f, QWidget *parent = Q_NULLPTR);
    void setDisabled(bool disabled);

    // Signals
    Signal1<double> Changed;
public slots:
    void OnChanged(double value);
    void OnSetValue(double value);
protected:
    DoubleEdit* value_edit_;
    QSlider* value_slide_;

    void SliderSet(double value);
    double SliderGet();

    double value_;
    double range_min_;
    double slider_total_;
    double slider_max_;
};

#endif // SLIDER_H
