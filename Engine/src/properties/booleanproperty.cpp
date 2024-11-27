/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "booleanproperty.h"

BooleanProperty::BooleanProperty(bool set) :
    Property(),
    value_(set)
{
}

bool BooleanProperty::Get() const {
    return value_;
}

void BooleanProperty::Set(bool value) {
    value_ = value;
    if (allow_signals_) ValueSet.Emit(value);
}

void BooleanProperty::SaveToYAML(YAML::Emitter& out) const {
    out << value_;
}

void BooleanProperty::LoadFromYAML(const YAML::Node& node) {
    Set(node.as<bool>());
}
