/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef QPROPERTYWIDGETITEM_H
#define QPROPERTYWIDGETITEM_H

#include <qtwidgets.h>
#include <properties.h>
#include <animation/curve.h>

class QAnimatableWidgetItem : public QTreeWidgetItem
{
public:
    Signal1<Curve*> PropertyUpdated;

    QAnimatableWidgetItem(const std::string& name, DoubleProperty* property);
    ~QAnimatableWidgetItem();
    DoubleProperty* GetProperty() { return property_; }
    std::string GetName() { return name_; }

private:
    void CurveChanged() { PropertyUpdated.Emit(dynamic_cast<Curve*>(property_->GetCurve())); }
    DoubleProperty* property_;
    std::string name_;
};
#endif // QPROPERTYWIDGETITEM_H
