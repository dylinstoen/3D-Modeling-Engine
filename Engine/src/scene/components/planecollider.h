/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef PLANECOLLIDER_H
#define PLANECOLLIDER_H

#include <scene/components/component.h>

class PlaneCollider : public Component {
public:
    DoubleProperty Width;
    DoubleProperty Height;
    DoubleProperty Restitution;

    PlaneCollider();

};

#endif // PLANECOLLIDER_H
