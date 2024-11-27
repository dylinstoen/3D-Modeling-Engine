/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "spherecollider.h"

REGISTER_COMPONENT(SphereCollider, SphereCollider)

SphereCollider::SphereCollider() :
    Radius(0.5f, 0.01f, 10.0f, 0.1f),
    Restitution(0.5f, 0.0f, 1.0f, 0.1f)
{
    AddProperty("Radius", &Radius);
    AddProperty("Restitution", &Restitution);
}
