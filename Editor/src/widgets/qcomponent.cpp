/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "qcomponent.h"
#include <inspector/inspector.h>

QSizePolicy QComponent::size_policy_ = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

QComponent::QComponent(const QString& name, InspectableItem& owner, QWidget *parent) :
    QGroupBox(name, parent),
    qname_(name),
    layout_(new QFormLayout(nullptr)),
    owner_(&owner)
{
    // TODO: Static constructor?
    size_policy_.setVerticalStretch(0);
    size_policy_.setHorizontalStretch(0);

    setAlignment(Qt::AlignHCenter);
    setSizePolicy(QComponent::size_policy_);

    setLayout(layout_);
    layout_->setFormAlignment(Qt::AlignCenter);
}

void QComponent::SetProperties(ObjectWithProperties* properties) {
    qDeleteAll(children());
    layout_ = new QFormLayout(nullptr);
    layout_->setFormAlignment(Qt::AlignCenter);
    for (auto& property : properties->GetProperties()) {
        Inspector::AddProperty(*layout_, property, properties->GetProperty(property), *owner_);
    }
    setLayout(layout_);
}
