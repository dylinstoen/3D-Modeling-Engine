/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef GIZMOMANAGER_H
#define GIZMOMANAGER_H
#include <animator.h>
#include <QObject>
#include <scene/translator.h>
#include <scene/rotator.h>
#include <scene/scaler.h>
#include <scene/scene.h>
#include <scene/scenecamera.h>

class GizmoManager : public QObject
{
public:
    GizmoManager();

    void InitializeGizmos(Scene* scene);
    void SetTransforms(SceneObject* scene_object, SceneCamera* camera, Space space = Space::World);
    SceneObject* GetClickedManipulator() { return clicked_gizmo_; }

public slots:
    void TranslationBegin(
            const SceneCamera* camera,
            glm::vec3 initial_position,
            uint64_t click_uid);
    void ObjectTranslated(int x, int y);
    void RotationBegin(
            const SceneCamera* camera,
            glm::vec3 initial_position,
            uint64_t click_uid);
    void ObjectRotated(int x, int y);
    void ScaleBegin(
            const SceneCamera* camera,
            glm::vec3 initial_position,
            uint64_t click_uid);
    void ObjectScaled(int x, int y);
    void OnObjectDeleted(SceneObject& obj);
    void OnObjectSelected(uint64_t uid);
    void SelectObject(SceneObject* scene_object);
    void ManipulationDone();

    void SetEnabled(bool enable) {
        manipulator_->SetEnabled(enable, false);
    }
    void SetTranslatorEnabled(bool enable) {
        translator_gizmo_->SetEnabled(enable, false);
        translate_center_->SetEnabled(enable && allow_free_translation_, false);
    }
    void SetRotatorEnabled(bool enable) {
        rotator_gizmo_->SetEnabled(enable, false);
        viewspace_rotator_gizmo_->SetEnabled(enable && allow_free_rotation_, false);
    }
    void SetScalerEnabled(bool enable) {
        scaler_gizmo_->SetEnabled(enable, false);
        scaler_center_->SetEnabled(enable && allow_uniform_scale_, false);
    }

    bool IsGizmo(uint64_t uid) {
        return uid2gizmo.find(uid) != uid2gizmo.end();
    }

protected:
    SceneObject* GetManipulatedObject();
    Scene* scene_;
    SceneObject* manipulator_;
    // References to individual manipulators
    SceneObject* manipulator_transform_;
    SceneObject* translator_gizmo_;
    SceneObject* translate_center_;
    SceneObject* translator_arrows_[3];
    SceneObject* translator_planes_[3];
    SceneObject* rotator_gizmo_;
    SceneObject* rotator_disks_[3];
    SceneObject* viewspace_rotator_gizmo_;
    SceneObject* scaler_gizmo_;
    SceneObject* scaler_center_;
    SceneObject* scaler_clubs_[3];

    Translator* translator_;
    Rotator* rotator_;
    Scaler* scaler_;
    glm::vec3 scale_axis;

    bool manipulating_;

    SceneObject* clicked_gizmo_;

    bool allow_free_translation_;
    bool allow_free_rotation_;
    bool allow_uniform_scale_;

    std::unordered_map<uint64_t, SceneObject*> uid2gizmo;
    std::unordered_map<uint64_t, glm::vec4> translation_planes;
    std::unordered_map<uint64_t, glm::vec3> translation_axes;
    std::unordered_map<uint64_t, glm::vec3> rotation_axes;
    std::unordered_map<uint64_t, glm::vec3> scale_axes;
};

#endif // GIZMOMANAGER_H
