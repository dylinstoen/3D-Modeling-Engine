/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "choiceproperty.h"

ChoiceProperty::ChoiceProperty(const std::initializer_list<std::string> &choices, int selected) :
    Property(),
    choices_(choices),
    current_index_(selected)
{

}

int ChoiceProperty::Get() const {
    if (choices_.size() == 0) return -1;
    else return current_index_;
}

std::string ChoiceProperty::GetLabel() const {
    if (choices_.size() == 0) return "";
    else return choices_[current_index_];
}

void ChoiceProperty::Set(int index) {
    current_index_ = index;
    if (current_index_ >= choices_.size()) current_index_ = choices_.size() - 1;

    if (allow_signals_) ValueSet.Emit(current_index_);
}

std::vector<std::string> ChoiceProperty::GetChoices() const {
    return std::vector<std::string>(choices_);
}

void ChoiceProperty::SaveToYAML(YAML::Emitter& out) const {
    out << current_index_;
}

void ChoiceProperty::LoadFromYAML(const YAML::Node& node) {
    Set(node.as<int>());
}
