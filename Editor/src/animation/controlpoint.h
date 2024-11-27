/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CONTROLPOINT_H
#define CONTROLPOINT_H

#include <animation/keyframe.h>
#include <widgets/QCustomPlot/qcustomplot.h>

class Curve;

// TODO: Perhaps instead of tracer, this should just have an anchor
class ControlPoint : public QCPItemEllipse, public Keyframe {
    Q_OBJECT
public:
    explicit ControlPoint(QCustomPlot& parent_plot, Curve* parent_curve, float t, float y);

    virtual void Set(float t, float y) override;

    Curve& GetParentCurve() { return *parent_curve_; }
protected:
    Curve* parent_curve_;
    QCPItemPosition* center_;
};

#endif // CONTROLPOINT_H
