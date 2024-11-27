/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "light.h"

REGISTER_COMPONENT(PointLight, Light)
REGISTER_COMPONENT(DirectionalLight, Light)
REGISTER_COMPONENT(AreaLight, Light)
REGISTER_COMPONENT(SpotLight, Light)

Light::Light() :
    Ambient(false, glm::vec4(0.f, 0.f, 0.f, 1.f)),
    Color(false)
{
    AddProperty("Color", &Color);
    AddProperty("Ambient", &Ambient);
}
