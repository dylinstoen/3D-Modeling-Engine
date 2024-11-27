/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef TRACKBALL_H
#define TRACKBALL_H

#include <animator.h>
#include <scene/scenecamera.h>

class Trackball
{
public:
    Trackball() : rotation_scale_(20) {}

    void SetRotationScale(double scale) { rotation_scale_ = scale; }
    void RotationBegin(float x, float y, float w, float h) {
        x = (x/w - 0.5f);
        y = (y/h - 0.5f);
        // Project screen-space coordinates onto a sphere
        rotation_begin_ = screen2sphere(x,y);
    }

    void TranslationBegin(int x, int y) {
        translation_begin_ = glm::vec3(x, y, 0.0f);
    }

    void TranslationUpdate(int dx, int dy, glm::vec3& out_translation) {
        glm::vec3 mouse_end = glm::vec3(dx, dy, 0.0f);
        out_translation = mouse_end - translation_begin_;
//        out_translation *= rotation_speed_;
        translation_begin_ = mouse_end;
    }

    void RotationUpdate(float x, float y, float w, float h, glm::vec3& out_axis, double& out_angle) {
        x = (x/w - 0.5f);
        y = (y/h - 0.5f);
        // Project screen-space coordinates onto a sphere
        glm::vec3 mouse_end = screen2sphere(x,y);
        // Rotation axis is perpendicular to the great circle containing the projected start and end points
        out_axis = glm::normalize(glm::cross(mouse_end, rotation_begin_));
        if (std::isnan(out_axis.x)) {
            out_axis = glm::vec3(1,0,0);
            out_angle = 0;
            return;
        }
        // The magnitude of the difference vector can be proportional to the rotation angle.
        out_angle = rotation_scale_*acos(std::min(std::max(glm::dot(mouse_end,rotation_begin_),-1.f),1.f));
        rotation_begin_ = mouse_end;
    }
protected:
    static glm::vec3 screen2sphere(float x, float y) {
        float r = 0.3f;
        float d = x*x + y*y;
        // For smoother behavior at the screen corners (where points project outside the
        // sphere), we want to transition to a smoother shape, such as the hyperbolic sheet
        // as suggested by https://www.khronos.org/opengl/wiki/Object_Mouse_Trackball
        if (d < r*r/2) return glm::normalize(glm::vec3(x, y, sqrt(r*r-std::min(d,r*r))));
        else return glm::normalize(glm::vec3(x, y, r*r/(2*sqrt(d))));
    }

    glm::vec3 rotation_begin_;
    glm::vec3 translation_begin_;
    double rotation_scale_;
};

#endif // TRACKBALL_H
