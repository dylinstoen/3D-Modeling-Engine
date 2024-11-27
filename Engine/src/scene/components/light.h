/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef LIGHT_H
#define LIGHT_H

#include <animator.h>
#include <scene/components/component.h>

// TODO: Only the first four lights of each type is passed to the shader right now
// Unity does lighting in multiple passes.

class Light : public Component {
public:
    ColorProperty Ambient;
    ColorProperty Color;

    virtual glm::vec3 GetIntensity() {
        return Color.GetRGB();
    }

    Light();
};

class AttenuatingLight : public Light {
public:
    DoubleProperty AttenA;
    DoubleProperty AttenB;
    DoubleProperty AttenC;

    AttenuatingLight() :
        Light(),
        AttenA(0.20, 0, 1, .01),
        AttenB(0.0, 0, 1, .01),
        AttenC(0.0, 0, 1, .01)\
    {
        AddProperty("Quad Atten", &AttenA);
        AddProperty("Linear Atten", &AttenB);
        AddProperty("Const Atten", &AttenC);
    }
};

class PointLight : public AttenuatingLight {
public:
    DoubleProperty TraceRadius;

    PointLight() :
        AttenuatingLight(),
        TraceRadius(0)
    {
        AddProperty("Radius (Trace)", &TraceRadius);
    }

};

class AreaLight : public AttenuatingLight {
public:
    //actually just scale it with the transform
    //DoubleProperty Width;
    //DoubleProperty Height;

    AreaLight() :
        AttenuatingLight()
        //Width(1),
        //Height(1)
    {
        //AddProperty("Width", &Width);
        //AddProperty("Height", &Height);
    }

};

/* EXTRA CREDIT: Implement Spotlights */
class SpotLight : public AttenuatingLight {
public:
    DoubleProperty Cutoff;
    DoubleProperty Falloff;
    SpotLight() :
        AttenuatingLight(),
        Cutoff(45.0, 0.0, 180.0, 1.0),
        Falloff(0.0, 0.0, 100.0, 1.0)
    {
        AddProperty("Cutoff Angle", &Cutoff);
        AddProperty("Falloff", &Falloff);
    }
};

// Uses the rotation as the light angle
class DirectionalLight : public Light {
public:
    DoubleProperty IntensityMultiplier;
    DoubleProperty TraceAngularSize;

    virtual glm::vec3 GetIntensity() {
        return (float)IntensityMultiplier.Get() * Color.GetRGB();
    }

    DirectionalLight() :
        Light(),
        IntensityMultiplier(1.0, 0.25, 10.0, 0.25),
        TraceAngularSize(0, 0, 1.5, 0.01)
    {
        AddProperty("Intensity Multiplier", &IntensityMultiplier);
        AddProperty("Angular Size (Trace)", &TraceAngularSize);
    }

};

#endif // LIGHT_H
