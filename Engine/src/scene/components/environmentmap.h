/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef ENVIRONMENTMAP_H
#define ENVIRONMENTMAP_H

#include <scene/components/component.h>
#include <resource/material.h>
#include <resource/texture.h>

class EnvironmentMap : public Component {
public:
    IntProperty Resolution;
    DoubleProperty NearPlane;
    DoubleProperty FarPlane;
    ResourceProperty<Material> RenderMaterial;

    EnvironmentMap(int resolution = 400, double near_plane = 0.1f, double far_plane = 100.0f);
    ~EnvironmentMap();

    // Utilities for rendering
    glm::mat4 GetViewMatrix(int face, glm::mat4 matrix);
    glm::mat4 GetProjection() { return projection_; }

    RenderableCubemap& GetCubemap() { return *cubemap_asset_; }

protected:
    RenderableCubemap *cubemap_asset_;

    void ResolutionChanged(int resolution);
    void CalculateProjection();
    void CalculateProjectionI(int) { CalculateProjection(); }
    void CalculateProjectionD(double) { CalculateProjection(); }

    glm::mat4 projection_;
};

#endif // ENVIRONMENTMAP_H
