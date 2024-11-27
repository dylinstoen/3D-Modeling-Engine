/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CYLINDERCOLLIDER_H
#define CYLINDERCOLLIDER_H

#include <scene/components/component.h>

class CylinderCollider : public Component {
public:
    DoubleProperty Diameter;
    DoubleProperty Height;
    DoubleProperty Restitution;

    CylinderCollider();

};

#endif // CYLINDERCOLLIDER_H
