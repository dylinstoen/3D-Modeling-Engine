/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "qanimtablewidgetitem.h"

QAnimatableWidgetItem::QAnimatableWidgetItem(const std::string& name, DoubleProperty* property) :
    property_(property),
    name_(name)
{
    setText(0, QString::fromStdString(name));
    if (property) property->CurveUpdated.Connect(this, &QAnimatableWidgetItem::CurveChanged);
}
QAnimatableWidgetItem::~QAnimatableWidgetItem() {
    if (property_) property_->CurveUpdated.Disconnect(this, &QAnimatableWidgetItem::CurveChanged);
}
