/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "rotator.h"

void Rotator::RotationBegin(
        glm::vec3 clicked_point,
        glm::vec3 axis,
        glm::vec3 object_center)
{

    trackball_.SetRotationScale(0.5);
    glm::vec4 p = camera_->GetProjection()*glm::vec4(clicked_point,1);
    glm::vec2 ndc = 1.f/p.w * p.xy;
    original_screencoords_ = 0.5f*camera_->GetScreenSize()*(ndc + glm::vec2(1,1));

    if (glm::dot(axis, axis) > 0) {
        use_trackball_ = false;
        glm::vec4 camspace_axis(axis, 0);
        camspace_axis = camera_->GetView()*camspace_axis;
        glm::vec4 p2 = camera_->GetProjection()*(glm::vec4(clicked_point,1) + camspace_axis);
        glm::vec2 ndc2 = 1.f/p2.w * p2.xy;
        glm::vec2 sc2 = 0.5f*camera_->GetScreenSize()*(ndc2 + glm::vec2(1,1));
        projected_axis_ = glm::normalize(original_screencoords_ - sc2);

        if (std::abs(glm::dot(camspace_axis, glm::vec4(0,0,1,0))) > 0.9) {
            circular_projection_ = true;
            glm::vec4 camspace_center = camera_->GetProjection()*camera_->GetView()*glm::vec4(object_center,1);
            glm::vec2 ndc_center = 1.f/camspace_center.w * camspace_center.xy;
            center_screencoords_ = 0.5f*camera_->GetScreenSize()*(ndc_center + glm::vec2(1,1));
            glm::vec2 d = original_screencoords_ - center_screencoords_;
            prev_angle_ = atan2(d.y, d.x)*180/M_PI;
        } else {
            circular_projection_ = false;
            prev_angle_ = 0;
        }
        rotation_axis_ = axis;
    } else {
        use_trackball_ = true;
        trackball_.RotationBegin(original_screencoords_.x, original_screencoords_.y, camera_->GetScreenSize().x, camera_->GetScreenSize().y);
    }
}

void Rotator::RotationUpdate(int dx, int dy, glm::vec3& out_axis, double& out_angle) {
    float angle;
    if (use_trackball_) {
        trackball_.RotationUpdate(dx, dy, camera_->GetScreenSize().x, camera_->GetScreenSize().y, out_axis, out_angle);
        out_angle = -out_angle;
    } else if (circular_projection_) {
        glm::vec2 L = glm::vec2(dx, dy) - center_screencoords_;
        out_axis = rotation_axis_;
        angle = atan2(L.y, L.x)*180/M_PI;
        out_angle =  angle - prev_angle_;
        prev_angle_ = angle;
    } else {
        glm::vec2 L = glm::vec2(dx, dy) - original_screencoords_;
        float distance = length(L - dot(L, projected_axis_)*projected_axis_);
        if (glm::cross(glm::vec3(L,0),glm::vec3(projected_axis_,0)).z > 0) distance = -distance;
        angle = distance*scale_*360;
        out_axis = rotation_axis_;
        out_angle =  angle - prev_angle_;
        prev_angle_ = angle;
    }
}
