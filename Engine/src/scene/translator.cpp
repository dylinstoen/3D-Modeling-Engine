/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "translator.h"

void Translator::TranslationBegin(
        glm::vec3 clicked_point,
        glm::vec4 translation_plane,
        glm::vec3 original_translation)
{
    original_point_ = clicked_point;
    original_translation_ = original_translation;
    translation_plane_ = translation_plane;

    if (dot(translation_plane_.xyz(), translation_plane_.xyz()) == 0) {
        glm::vec3 eye = camera_->GetTransform()*glm::vec4(0,0,0,1);
        glm::vec4 towards = camera_->GetTransform()*glm::vec4(0,0,-1,0);
        glm::vec3 p0 = eye + -original_point_.z*towards.xyz();
        translation_plane_ = glm::vec4(towards.xyz(), -dot(towards.xyz(), p0));
    }

    prev_translation_ = glm::vec3(0,0,0);
    axis_translation_ = false;
}

void Translator::AxisTranslationBegin(
        glm::vec3 clicked_point,
        glm::vec3 translation_axis,
        glm::vec3 original_translation)
{
    original_point_ = clicked_point;
    original_translation_ = original_translation;
    translation_plane_ = glm::vec4(translation_axis,0);
    prev_translation_ = glm::vec3(0,0,0);
    axis_translation_ = true;
}

glm::vec3 Translator::RayPlaneIntersection(glm::vec3 p, glm::vec3 v, glm::vec4 plane) {
    float t = -(plane.w + dot(plane.xyz(), p))/dot(plane.xyz(), v);
    return p + t*v;
}

glm::vec3 Translator::ClosestPointOnLine(glm::vec3 p, glm::vec3 v, glm::vec3 point, glm::vec3 axis) {
    // Ref: http://geomalgorithms.com/a07-_distance.html
    glm::vec3 w = p - point;
    float b = glm::dot(v, axis);
    float t = (glm::dot(axis,w) - glm::dot(v,axis)*glm::dot(v,w))/(1 - b*b);
    return point + t*axis;
}

glm::vec3 Translator::ClosestPointOnLine(glm::vec3 p, glm::vec3 point, glm::vec3 axis) {
    return point + dot(p-point, axis)*axis;
}

void Translator::TranslationUpdate(int dx, int dy, glm::vec3& out_translation) {
    glm::vec3 p, v;
    camera_->GetRay(glm::vec2(dx, dy), p, v);
    glm::vec3 new_position;
    glm::vec4 original_position = camera_->GetTransform()*glm::vec4(original_point_,1);
    if (axis_translation_) {
        if (std::abs(glm::dot(v, translation_plane_.xyz())) > 0.99) {
            new_position = original_position.xyz();
        } else {
            new_position = ClosestPointOnLine(p, v, original_position, translation_plane_.xyz());
        }
    } else {
        new_position = RayPlaneIntersection(p, v, translation_plane_);
    }
    glm::vec3 total_translation = new_position - original_position.xyz;
    out_translation = total_translation - prev_translation_;
    prev_translation_ = total_translation;
}
