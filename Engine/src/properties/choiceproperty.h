/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CHOICEPROPERTY_H
#define CHOICEPROPERTY_H

#include <properties/property.h>

class ChoiceProperty : public Property
{
public:
    Signal1<int> ValueSet;

    ChoiceProperty(const std::initializer_list<std::string>& choices, int selected = 0);

    int Get() const;
    std::string GetLabel() const;
    void Set(int index);

    virtual void SaveToYAML(YAML::Emitter& out) const override;
    virtual void LoadFromYAML(const YAML::Node& node) override;

    std::vector<std::string> GetChoices() const;
private:
    std::vector<std::string> choices_;
    size_t current_index_;
};

#endif // CHOICEPROPERTY_H
