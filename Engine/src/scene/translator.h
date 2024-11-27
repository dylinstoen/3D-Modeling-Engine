/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef TRANSLATOR_H
#define TRANSLATOR_H
#include <animator.h>
#include <scene/scenecamera.h>
#include <scene/sceneobject.h>

class Translator {
public:
    Translator(const SceneCamera* camera=NULL) : camera_(camera) {}
    void SetCamera(const SceneCamera* camera) { camera_ = camera; }

    // Takes clicked point in camera space,
    // translation plane in world space,
    // and original translation of object in world space
    void TranslationBegin(
            glm::vec3 clicked_point,
            glm::vec4 translation_plane,
            glm::vec3 original_translation=glm::vec3(0,0,0)
            );

    void AxisTranslationBegin(
            glm::vec3 clicked_point,
            glm::vec3 translation_axis,
            glm::vec3 original_translation=glm::vec3(0,0,0)
            );

    // Returns translation in world space
    void TranslationUpdate(int dx, int dy, glm::vec3& out_translation);

    glm::vec2 GetOriginalScreenCoords() const {
        glm::vec4 p = camera_->GetProjection()*glm::vec4(original_point_,1);
        glm::vec2 ndc = 1.f/p.w * p.xy;
        return 0.5f*camera_->GetScreenSize()*(ndc + glm::vec2(1,1));
    }
    glm::vec3 GetOriginalPosition() const {
        return (camera_->GetTransform()*glm::vec4(original_point_,1)).xyz;
    }
    glm::vec3 GetNewPosition() const {
        return prev_translation_ + GetOriginalPosition();
    }

    // Utility functions
    static glm::vec3 RayPlaneIntersection(glm::vec3 p, glm::vec3 v, glm::vec4 plane);
    static glm::vec3 ClosestPointOnLine(glm::vec3 p, glm::vec3 v, glm::vec3 axis, glm::vec3 point);
    static glm::vec3 ClosestPointOnLine(glm::vec3 p, glm::vec3 axis, glm::vec3 point);

private:
    glm::vec3 original_point_; // clicked point in camera space
    glm::vec4 translation_plane_;
    glm::vec3 original_translation_;
    glm::vec3 prev_translation_;
    const SceneCamera* camera_;
    bool axis_translation_;
};

#endif // TRANSLATOR_H
