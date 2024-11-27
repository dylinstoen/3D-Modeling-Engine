/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SCALER_H
#define SCALER_H
#include <animator.h>
#include <scene/translator.h>
#include <scene/scenecamera.h>
#include <scene/sceneobject.h>

class Scaler
{
public:
    Scaler(const SceneCamera* camera=NULL) : t(camera), freescale(false) {}
    void SetCamera(const SceneCamera* camera) { t.SetCamera(camera); }

    // Takes clicked point in camera space,
    // scale plane in world space,
    // and original scale of object in world space
    void ScaleBegin(
            glm::vec3 clicked_point,
            glm::vec3 center,
            glm::vec3 original_scale=glm::vec3(1,1,1)
            );

    void AxisScaleBegin(
            glm::vec3 clicked_point,
            glm::vec3 center,
            glm::vec3 scale_axis,
            glm::vec3 original_scale=glm::vec3(1,1,1)
            );
    void PlaneScaleBegin(
            glm::vec3 clicked_point,
            glm::vec3 center,
            glm::vec4 scale_plane,
            glm::vec3 original_scale=glm::vec3(1,1,1)
            );

    // Returns scale in object space
    void ScaleUpdate(int dx, int dy, glm::vec3& out_translation);

    glm::vec3 GetOriginalScale() const { return original_scale_; }

private:
    glm::vec3 original_center_;
    glm::vec3 original_scale_;
    Translator t;
    bool freescale;
};

#endif // SCALER_H
