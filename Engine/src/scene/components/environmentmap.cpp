/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "environmentmap.h"
#include <glm/gtx/transform.hpp>

REGISTER_COMPONENT(EnvironmentMap, EnvironmentMap)

EnvironmentMap::EnvironmentMap(int resolution, double near_plane, double far_plane) :
    Resolution(true, resolution),
    NearPlane(near_plane),
    FarPlane(far_plane),
    RenderMaterial(AssetType::Material)
{
    AddProperty("Resolution (px)", &Resolution);
    AddProperty("Near Plane", &NearPlane);
    AddProperty("Far Plane", &FarPlane);
    AddProperty("Render Material", &RenderMaterial);

    Resolution.ValueChanged.Connect(this, &EnvironmentMap::ResolutionChanged);
    NearPlane.ValueChanged.Connect(this, &EnvironmentMap::CalculateProjectionD);
    FarPlane.ValueChanged.Connect(this, &EnvironmentMap::CalculateProjectionD);

    cubemap_asset_ = new RenderableCubemap("Environment Map", resolution);

    CalculateProjection();
}

EnvironmentMap::~EnvironmentMap() {
    delete cubemap_asset_;
}

glm::mat4 EnvironmentMap::GetViewMatrix(int face, glm::mat4 matrix) {
    // Environment map faces must be aligned according to world coordinates
    // Take translation from current view matrix
    glm::mat4 pos(1.f);
    for (int i = 0; i < 3; i++) {
        pos[i][3] = -matrix[i][3];
        pos[3][i] = -matrix[3][i];
    }

    // Compute appropriate rotation matrix for face
    glm::mat4 rot(1.f);
    switch(face) {
        case 0: // Positive X
            rot = glm::rotate(glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))*
                  glm::rotate(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        case 1: // Negative X
            rot = glm::rotate(glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))*
                  glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
            break;
        case 2: // Positive Y
            rot = glm::rotate(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
            break;
        case 3: // Negative Y
            rot = glm::rotate(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
            break;
        case 4: // Positive Z
            rot = glm::rotate(glm::radians(180.f), glm::vec3(1.f, 0.f, 0.f));
            break;
        case 5: // Negative Z, default
            rot = glm::rotate(glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));
            break;
    }

    return rot*pos;
}

void EnvironmentMap::ResolutionChanged(int resolution) {
    cubemap_asset_->SetResolution(resolution);
    CalculateProjection();
}

void EnvironmentMap::CalculateProjection() {
    projection_ = glm::perspective(glm::radians(90.f), 1.f, (float) NearPlane.Get(), (float) FarPlane.Get());
}
