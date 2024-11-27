/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef VEC3PROPERTY_H
#define VEC3PROPERTY_H

#include <properties/doubleproperty.h>
#include <properties/propertygroup.h>

class Vec3Property : public PropertyGroup {
public:
    Signal1<glm::vec3> ValueChanged;

    Vec3Property(glm::vec3 value = glm::vec3(0, 0, 0));
    DoubleProperty& GetPropertyX();
    DoubleProperty& GetPropertyY();
    DoubleProperty& GetPropertyZ();
    DoubleProperty* GetProperty(int i) { return i==0 ? &X : (i==1? &Y : (i==2? &Z : nullptr)); }

    glm::vec3 Get() const;
    void Set(glm::vec3 value);

private:
    glm::vec3 value_;
    // For animation purposes:
    // We can't animate a Vec3, but we can animate 3 doubles.
    DoubleProperty X;
    DoubleProperty Y;
    DoubleProperty Z;
    void OnChangedX(double x);
    void OnChangedY(double y);
    void OnChangedZ(double z);
    void OnHiddenChangedXYZ(bool);
};

#endif // VEC3PROPERTY_H
