/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "transform.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <meshprocessing.h>

REGISTER_COMPONENT(Transform, Transform)

Transform::Transform() :
    Translation(),
    Rotation(),
    Scale(glm::vec3(1.0f, 1.0f, 1.0f)),
    matrix_override_(false),
    tracking_vertex_of_(nullptr),
    tracking_vertex_id_(0)
{
    AddProperty("Translation", &Translation);
    AddProperty("Rotation", &Rotation);
    AddProperty("Scale", &Scale);

    Translation.ValueChanged.Connect(this, &Transform::Update);
    Scale.ValueChanged.Connect(this, &Transform::Update);
    Rotation.ValueChanged.Connect(this, &Transform::Update);
}

void Transform::CopyFrom(const Transform& transform) {
    Translation.Set(transform.Translation.Get());
    Rotation.Set(transform.Rotation.Get());
    Scale.Set(transform.Scale.Get());
}

void Transform::SetFromMatrix(glm::mat4 m) {
    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat orientation;
    glm::decompose(m, scale, orientation, translation, skew, perspective);
    Translation.Set(translation);
    Scale.Set(scale);
    Rotation.Set(glm::degrees(-glm::eulerAngles(orientation)));
}

void Transform::ForceMatrix(glm::mat4 m) {
    matrix_override_ = true;
    matrix_ = m;
}

glm::mat4 Transform::GetMatrix() const {
    if (matrix_override_) return matrix_;
    else return cached_;
}

void Transform::Update(glm::vec3) {

    glm::mat4 translate = glm::translate(glm::mat4(), Translation.Get());
    glm::mat4 rotate = glm::eulerAngleXYZ(glm::radians(Rotation.Get().x), glm::radians(Rotation.Get().y), glm::radians(Rotation.Get().z));
    rotation_ = glm::quat_cast(rotate);
    glm::mat4 scale = glm::scale(glm::mat4(), Scale.Get());
    cached_ = translate * rotate * scale;

    //Ensure mesh is valid?
    if (tracking_vertex_of_) {
        const std::vector<float>& pos_vector = tracking_vertex_of_->GetPositions();
        std::vector<float> new_pos_vector = pos_vector;
        new_pos_vector[tracking_vertex_id_*3] = Translation.Get().x;
        new_pos_vector[tracking_vertex_id_*3+1] = Translation.Get().y;
        new_pos_vector[tracking_vertex_id_*3+2] = Translation.Get().z;

        tracking_vertex_of_->SetPositions(new_pos_vector);
        MeshProcessing::ComputeNormals(*tracking_vertex_of_);
    }
}

void Transform::SetVertexTracking(Mesh* mesh, int vertex) {
    tracking_vertex_of_ = mesh;
    tracking_vertex_id_ = vertex;

    if (mesh) {
        const std::vector<float>& pos_vector = mesh->GetPositions();
        glm::vec3 vertex_pos(pos_vector[vertex*3], pos_vector[vertex*3+1], pos_vector[vertex*3+2]);
        Translation.Set(vertex_pos);
    }
}

void Transform::Translate(glm::vec3 translation, Space relative_frame) {
    if (relative_frame == Space::Local) {
        // This basically rotates the translation so it's applied locally
        translation = (glm::toMat4(rotation_) * glm::vec4(translation, 1.0)).xyz;
    }

    glm::mat4 matrix = glm::translate(glm::mat4(), Translation.Get());
    Translation.Set(glm::vec3(matrix * glm::vec4(translation, 1.0)));
}

void Transform::Rotate(glm::vec3 axis, double angle) {
    if (glm::dot(axis, axis) < std::numeric_limits<float>::epsilon() || std::abs(angle) < std::numeric_limits<double>::epsilon()) return;

    // Get current rotation matrix
    glm::mat4 current_rotation = glm::eulerAngleXYZ(glm::radians(Rotation.Get().x), glm::radians(Rotation.Get().y), glm::radians(Rotation.Get().z));
    // Apply new rotation
    glm::mat4 new_rotation = glm::rotate(current_rotation, float(glm::radians(angle)), axis);

    // Set the values
    float x, y, z;
    glm::extractEulerAngleXYZ(new_rotation, x, y, z);
    Rotation.Set(glm::degrees(glm::vec3(x, y, z)));
}
