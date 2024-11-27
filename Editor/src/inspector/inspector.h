/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef INSPECTOR_H
#define INSPECTOR_H

#include <QStackedWidget>
#include <inspector/inspectableitem.h>
#include <qtwidgets.h>
#include <properties.h>

class Inspector : public QStackedWidget {
    Q_OBJECT

public:
    Inspector();
    static void AddProperty(QFormLayout& layout, std::string name, Property* property, InspectableItem& owner);

public slots:
    void OnAddInspectable(InspectableItem& item);
    void OnRemoveInspectable(InspectableItem& item);
    void OnSelectInspectable(InspectableItem* item);

private:
    QWidget* blank_;
};

#endif // INSPECTOR_H
