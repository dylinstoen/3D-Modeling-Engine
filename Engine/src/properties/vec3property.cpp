/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "vec3property.h"
#include "yamlextensions.h"

Vec3Property::Vec3Property(glm::vec3 value) :
    PropertyGroup(),
    value_(value),
    X(),
    Y(),
    Z()
{
    // For animation purposes only
    AddProperty("X",&X);
    AddProperty("Y",&Y);
    AddProperty("Z",&Z);

    // Listen for updates from the animatable properties
    X.ValueChanged.Connect(this, &Vec3Property::OnChangedX);
    Y.ValueChanged.Connect(this, &Vec3Property::OnChangedY);
    Z.ValueChanged.Connect(this, &Vec3Property::OnChangedZ);

    // Whenever all three values are hidden, hide this too.
    // Whenever a value is unhidden, unhide this too
    X.HiddenChanged.Connect(this, &Vec3Property::OnHiddenChangedXYZ);
    Y.HiddenChanged.Connect(this, &Vec3Property::OnHiddenChangedXYZ);
    Z.HiddenChanged.Connect(this, &Vec3Property::OnHiddenChangedXYZ);

    Set(value);
}

DoubleProperty &Vec3Property::GetPropertyX() {
    return X;
}

DoubleProperty &Vec3Property::GetPropertyY() {
    return Y;
}

DoubleProperty &Vec3Property::GetPropertyZ() {
    return Z;
}

glm::vec3 Vec3Property::Get() const { return value_; }

void Vec3Property::Set(glm::vec3 value) {
    value_ = value;

    // Update the animatable properties
    X.BlockSignals();
    Y.BlockSignals();
    Z.BlockSignals();
    X.Set(value_.x);
    Y.Set(value_.y);
    Z.Set(value_.z);
    X.UnblockSignals();
    Y.UnblockSignals();
    Z.UnblockSignals();

    if (allow_signals_) ValueChanged.Emit(value_);
}

void Vec3Property::OnChangedX(double x) {
    glm::vec3 value = Get();
    value.x = x;
    Set(value);
}

void Vec3Property::OnChangedY(double y) {
    glm::vec3 value = Get();
    value.y = y;
    Set(value);
}

void Vec3Property::OnChangedZ(double z) {
    glm::vec3 value = Get();
    value.z = z;
    Set(value);
}

void Vec3Property::OnHiddenChangedXYZ(bool) {
    SetHidden(X.IsHidden() && Y.IsHidden() && Z.IsHidden());
}

