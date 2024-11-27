/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef BOOLEANPROPERTY_H
#define BOOLEANPROPERTY_H

#include <properties/property.h>

class BooleanProperty : public Property {
public:
    Signal1<bool> ValueSet;

    BooleanProperty(bool set=false);

    bool Get() const;
    void Set(bool value);
    virtual void SaveToYAML(YAML::Emitter& out) const override;
    virtual void LoadFromYAML(const YAML::Node& node) override;

private:
    bool value_;
};

#endif // BOOLEANPROPERTY_H
