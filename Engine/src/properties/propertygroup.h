/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef PROPERTYGROUP_H
#define PROPERTYGROUP_H

#include <properties/property.h>

class PropertyGroup : public ObjectWithProperties, public Property
{
public:
    Signal1<ObjectWithProperties*> ValueSet;
    //PropertyGroup(std::vector<std::pair<std::string,Property*>> properties = std::vector<std::pair<std::string,Property*>>());
  //  ~PropertyGroup();
    PropertyGroup() : ObjectWithProperties(), Property() {}

    void PropertiesChanged() {
        ValueSet.Emit(this);
    }

    virtual void SaveToYAML(YAML::Emitter& out) const {
        ObjectWithProperties::SaveToYAML(out);
    }

    virtual void LoadFromYAML(const YAML::Node& node) {
       ObjectWithProperties::LoadFromYAML(node);
    }
};


#endif // PROPERTYGROUP_H
