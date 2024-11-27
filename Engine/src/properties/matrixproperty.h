/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef MATRIXPROPERTY_H
#define MATRIXPROPERTY_H

#include <properties/property.h>

class Mat4Property : public Property
{
public:
    Signal1<glm::mat4> ValueSet;

    Mat4Property(glm::mat4 value = glm::mat4());

    glm::mat4 Get() const;
    void Set(glm::mat4 value);

    virtual void SaveToYAML(YAML::Emitter& out) const override;
    virtual void LoadFromYAML(const YAML::Node& node) override;

private:
    glm::mat4 value_;
};

#endif // MATRIXPROPERTY_H
