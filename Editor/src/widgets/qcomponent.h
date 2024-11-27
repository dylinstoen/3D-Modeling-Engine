/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef QCOMPONENT_H
#define QCOMPONENT_H

#include <animator.h>
#include <inspector/inspectableitem.h>
#include <QGroupBox>
#include <QSizePolicy>
#include <QFormLayout>

class QComponent : public QGroupBox {
public:
    QComponent(const QString& name, InspectableItem& owner, QWidget *parent = Q_NULLPTR);

    void SetProperties(ObjectWithProperties* properties);
private:
    QString qname_;
    QFormLayout* layout_;
    InspectableItem* owner_;
    static QSizePolicy size_policy_;
};

#endif // QCOMPONENT_H
