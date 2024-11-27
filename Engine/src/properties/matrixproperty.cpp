/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "matrixproperty.h"
#include "yamlextensions.h"

Mat4Property::Mat4Property(glm::mat4 value) :
    Property(),
    value_(value)
{
}

glm::mat4 Mat4Property::Get() const {
    return value_;
}

void Mat4Property::Set(glm::mat4 value) {
    value_ = value;
    if (allow_signals_) ValueSet.Emit(value);
}

void Mat4Property::SaveToYAML(YAML::Emitter& out) const {
    out << value_;
}

void Mat4Property::LoadFromYAML(const YAML::Node& node) {
    Set(node.as<glm::mat4>());
}
