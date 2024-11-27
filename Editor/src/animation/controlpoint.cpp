/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "controlpoint.h"

ControlPoint::ControlPoint(QCustomPlot &parent_plot, Curve *parent_curve, float t, float y) :
    QCPItemEllipse(&parent_plot),
    Keyframe(t, y),
    parent_curve_(parent_curve),
    center_(createPosition("Center"))
{
    const double radius = 5;
    //        setClipToAxisRect(false);
    center_->setCoords(t, y);
    topLeft->setParentAnchor(center_);
    bottomRight->setParentAnchor(center_);
    topLeft->setType(QCPItemPosition::ptAbsolute);
    bottomRight->setType(QCPItemPosition::ptAbsolute);
    topLeft->setCoords(-radius, -radius);
    bottomRight->setCoords(radius, radius);

    setBrush(QBrush(QColor(255, 122, 0, 255)));
    setSelectedBrush(QBrush(QColor(122, 255, 0, 255)));
    setSelectable(true);
    setPen(QPen(Qt::black));
    setSelectedPen(QPen(Qt::red, 3));

    // Put it in the "overlay" layer
    QCPLayer* overlay_layer = parent_plot.layer("overlay");
    setLayer(overlay_layer);
}

void ControlPoint::Set(float t, float y) {
    Keyframe::Set(t, y);
    center_->setCoords(t, y);
}
