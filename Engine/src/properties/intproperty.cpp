/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "intproperty.h"

IntProperty::IntProperty(bool is_unsigned, int value) :
    Property(),
    value_(value),
    is_unsigned_(is_unsigned)
{

}

int IntProperty::Get() const {
    return value_;
}

void IntProperty::Set(int value) {
    value_ = value;
    if (is_unsigned_ && (value_ < 0)) value_ = 0;
    if (allow_signals_) ValueChanged.Emit(value);
}

void IntProperty::SaveToYAML(YAML::Emitter& out) const {
    out << value_;
}

void IntProperty::LoadFromYAML(const YAML::Node& node) {
    Set(node.as<int>());
}
