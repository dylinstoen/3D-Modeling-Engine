/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef COLORPROPERTY_H
#define COLORPROPERTY_H

#include <properties/propertygroup.h>
#include <properties/doubleproperty.h>

class ColorProperty : public PropertyGroup {
public:
    Signal1<glm::vec4> ValueSet;

    ColorProperty(bool use_alpha = true, glm::vec4 value = glm::vec4(1.0, 1.0, 1.0, 1.0));

    glm::vec4 Get() const;
    glm::vec3 GetRGB() const;
    void Set(glm::vec4 value);
    void Set(glm::vec3 value);

    bool UsesAlpha() const;
private:
    bool use_alpha_;
    glm::vec4 value_;
    // For animation purposes:
    // We can't animate a Vec4, but we can animate 4 doubles.
    DoubleProperty R;
    DoubleProperty G;
    DoubleProperty B;
    DoubleProperty A;
    void OnRChanged(double r);
    void OnGChanged(double g);
    void OnBChanged(double b);
    void OnAChanged(double a);
};

#endif // COLORPROPERTY_H
