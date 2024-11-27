/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "colorproperty.h"
#include "yamlextensions.h"

ColorProperty::ColorProperty(bool use_alpha, glm::vec4 value) :
    PropertyGroup(),
    use_alpha_(use_alpha),
    value_(value),
    R(),
    G(),
    B(),
    A()
{
    AddProperty("R",&R);
    AddProperty("G",&G);
    AddProperty("B",&B);
    AddProperty("A",&A);

    // Listen for updates from the animatable properties
    R.ValueChanged.Connect(this, &ColorProperty::OnRChanged);
    G.ValueChanged.Connect(this, &ColorProperty::OnGChanged);
    B.ValueChanged.Connect(this, &ColorProperty::OnBChanged);
    A.ValueChanged.Connect(this, &ColorProperty::OnAChanged);

    Set(value);
}

glm::vec4 ColorProperty::Get() const { return value_; }
glm::vec3 ColorProperty::GetRGB() const { return value_.xyz; }

void ColorProperty::Set(glm::vec3 value) {
    Set(glm::vec4(value.rgb, 1.0));
}

void ColorProperty::Set(glm::vec4 value) {
    if (use_alpha_) value_ = value;
    else value_ = glm::vec4(value.rgb, 1.0);

    // Update the animatable properties
    R.BlockSignals();
    G.BlockSignals();
    B.BlockSignals();
    A.BlockSignals();
    R.Set(value_.r);
    G.Set(value_.g);
    B.Set(value_.b);
    A.Set(value_.a);
    R.UnblockSignals();
    G.UnblockSignals();
    B.UnblockSignals();
    A.UnblockSignals();

    if (allow_signals_) ValueSet.Emit(value_);
}

bool ColorProperty::UsesAlpha() const { return use_alpha_; }

void ColorProperty::OnRChanged(double r) {
    glm::vec4 value = Get();
    value.r = r;
    Set(value);
}

void ColorProperty::OnGChanged(double g) {
    glm::vec4 value = Get();
    value.g = g;
    Set(value);
}

void ColorProperty::OnBChanged(double b) {
    glm::vec4 value = Get();
    value.b = b;
    Set(value);
}

void ColorProperty::OnAChanged(double a) {
    glm::vec4 value = Get();
    value.a = a;
    Set(value);
}
