/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <vectors.h>

// Pair<t, y> where t represents time and y represents the value in time
class Keyframe {
public:
    Keyframe(float t, float y) : point_(t, y) { }
    glm::vec2 Get() const { return point_; }
    virtual void Set(float t, float y) { point_.x = t; point_.y = y; }
protected:
    glm::vec2 point_;
};

#endif // KEYFRAME_H
