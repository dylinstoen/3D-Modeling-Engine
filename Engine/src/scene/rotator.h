/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef ROTATOR_H
#define ROTATOR_H
#include <animator.h>
#include <scene/scenecamera.h>
#include <scene/sceneobject.h>
#include <scene/trackball.h>

class Rotator
{
public:
    Rotator(const SceneCamera* camera=NULL) : scale_(0.001f), camera_(camera) {}
    void SetCamera(const SceneCamera* camera) { camera_ = camera; }
    void SetRotationScale(float scale) { scale_ = scale; }

    // Takes clicked point in camera space,
    // axis of rotation,
    // and original rotation of object in world space
    void RotationBegin(
            glm::vec3 clicked_point,
            glm::vec3 axis,
            glm::vec3 object_center = glm::vec3(0,0,0)
            );

    // Returns rotation in world space
    void RotationUpdate(int dx, int dy, glm::vec3& out_axis, double& out_angle);

    glm::vec3 GetRotationAxis() const { return rotation_axis_; }

private:
    glm::vec2 projected_axis_;
    glm::vec2 original_screencoords_;
    glm::vec2 center_screencoords_;
    glm::vec3 rotation_axis_;
    Trackball trackball_;
    float prev_angle_;
    float scale_;
    const SceneCamera* camera_;
    bool circular_projection_;
    bool use_trackball_;
};

#endif // ROTATOR_H
