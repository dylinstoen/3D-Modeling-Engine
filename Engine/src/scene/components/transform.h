/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <animator.h>
#include <scene/components/component.h>
#include <glm/gtx/quaternion.hpp>

class Mesh;

class Transform : public Component {
public:
    Vec3Property Translation;
    Vec3Property Rotation;
    Vec3Property Scale;

    Transform();
    void CopyFrom(const Transform& transform);

    // Sets the translation, rotation, and scale based on the given transformation matrix
    void SetFromMatrix(glm::mat4 m);
    void ForceMatrix(glm::mat4 m);

    // Returns the post-multiplied transformation matrix
    glm::mat4 GetMatrix() const;

    // Applies a translation <X, Y, Z> relative to either Local Space or World Space
    void Translate(glm::vec3 translation, Space relative_frame = Space::Local);
    // Applies a rotation of angle degrees about the axis <X, Y, Z>
    void Rotate(glm::vec3 axis, double angle);

    void BlockSignals() {
        Translation.BlockSignals();
        Rotation.BlockSignals();
        Scale.BlockSignals();
    }

    void UnblockSignals() {
        Translation.UnblockSignals();
        Rotation.UnblockSignals();
        Scale.UnblockSignals();
    }

    glm::vec3 GetForward() {
        return glm::normalize(rotation_ * glm::vec3(0,0,1));
    }

    void SetVertexTracking(Mesh* mesh, int vertex);
private:
    // Store the rotation as a quaternion for easier math
    glm::quat rotation_;
    glm::mat4 matrix_;
    glm::mat4 cached_;
    bool matrix_override_;

    Mesh* tracking_vertex_of_;
    int tracking_vertex_id_;

    void Update(glm::vec3);
};

#endif // TRANSFORM_H
