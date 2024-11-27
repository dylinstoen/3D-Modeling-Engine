/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "gizmomanager.h"

GizmoManager::GizmoManager()
    : translator_(new Translator),
      rotator_(new Rotator),
      scaler_(new Scaler),
      manipulating_(false),
      clicked_gizmo_(nullptr),
      allow_free_translation_(true),
      allow_free_rotation_(true),
      allow_uniform_scale_(true)
{

}

template<typename T>
static void recursive_add_map(std::unordered_map<uint64_t, T>& m, SceneObject* n, T v) {
    m[n->GetUID()] = v;
    std::vector<SceneObject*> children = n->GetChildren();
    for (auto child : n->GetChildren()) recursive_add_map(m, child, v);
}

static void recursive_set_material(SceneObject* n, Material* m) {
    if (n->GetComponent<Geometry>() != nullptr) {
        n->GetComponent<Geometry>()->RenderMaterial.Set(m);
    }
    for (auto child : n->GetChildren()) recursive_set_material(child, m);
}

void GizmoManager::InitializeGizmos(Scene *scene) {
    scene_ = scene;
    clicked_gizmo_ = nullptr;
    uid2gizmo.clear();
    translation_planes.clear();
    translation_axes.clear();
    rotation_axes.clear();
    scale_axes.clear();
    AssetManager& asset_manager = scene->GetAssetManager();

    // ------- Create default unlit materials -------
    Material* red_material = asset_manager.GetMaterial("_internal Unlit Red");
    Material* green_material = asset_manager.GetMaterial("_internal Unlit Green");
    Material* blue_material = asset_manager.GetMaterial("_internal Unlit Blue");
    Material* yellow_material = asset_manager.GetMaterial("_internal Unlit Yellow");
    Material* gray_material = asset_manager.GetMaterial("_internal Unlit Gray");

    scene->LockSignal();
    // ------- Create translation gizmo -------
    SceneObject& arrow_x = scene->CreateArrow("_gizmo Arrow X");
    arrow_x.GetTransform().Rotation.Set(glm::vec3(0.0f, 0.0f, -90.0f));
    recursive_add_map<glm::vec3>(translation_axes, &arrow_x, glm::vec3(1,0,0));
    recursive_set_material(&arrow_x, red_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &arrow_x, &arrow_x);

    SceneObject& arrow_y = scene->CreateArrow("_gizmo Arrow Y");
    recursive_add_map<glm::vec3>(translation_axes, &arrow_y, glm::vec3(0,1,0));
    recursive_set_material(&arrow_y, green_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &arrow_y, &arrow_y);

    SceneObject& arrow_z = scene->CreateArrow("_gizmo Arrow Z");
    arrow_z.GetTransform().Rotation.Set(glm::vec3(90.0f, 0.0f, 0.0f));
    recursive_add_map<glm::vec3>(translation_axes, &arrow_z, glm::vec3(0,0,1));
    recursive_set_material(&arrow_z, blue_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &arrow_z, &arrow_z);

    SceneObject& plane_xy = scene->CreatePlane("_gizmo Plane XY");
    plane_xy.GetTransform().Scale.Set(glm::vec3(0.1, 0.1, 0.1));
    plane_xy.GetTransform().Translation.Set(glm::vec3(0.125, 0.125, 0));
    recursive_add_map<glm::vec4>(translation_planes, &plane_xy, glm::vec4(0,0,1,0));
    recursive_set_material(&plane_xy, gray_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &plane_xy, &plane_xy);

    SceneObject& plane_xz = scene->CreatePlane("_gizmo Plane XZ");
    plane_xz.GetTransform().Scale.Set(glm::vec3(0.1, 0.1, 0.1));
    plane_xz.GetTransform().Translation.Set(glm::vec3(0.125, 0, 0.125));
    plane_xz.GetTransform().Rotation.Set(glm::vec3(90,0,0));
    recursive_add_map<glm::vec4>(translation_planes, &plane_xz, glm::vec4(0,1,0,0));
    recursive_set_material(&plane_xz, gray_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &plane_xz, &plane_xz);

    SceneObject& plane_yz = scene->CreatePlane("_gizmo Plane YZ");
    plane_yz.GetTransform().Scale.Set(glm::vec3(0.1, 0.1, 0.1));
    plane_yz.GetTransform().Translation.Set(glm::vec3(0, 0.125, 0.125));
    plane_yz.GetTransform().Rotation.Set(glm::vec3(0,90,0));
    recursive_add_map<glm::vec4>(translation_planes, &plane_yz, glm::vec4(1,0,0,0));
    recursive_set_material(&plane_yz, gray_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &plane_yz, &plane_yz);

    SceneObject& translate_center = scene->CreateMesh("_gizmo Translate Center", "Cube");
    translate_center.GetTransform().Scale.Set(glm::vec3(0.04f, 0.04f, 0.04f));
    recursive_set_material(&translate_center, gray_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &translate_center, &translate_center);

    SceneObject& translator = scene->CreateSceneObject("_gizmo Translator");
    translate_center.SetParent(translator);
    translate_center_ = &translate_center;
    arrow_x.SetParent(translator);
    arrow_y.SetParent(translator);
    arrow_z.SetParent(translator);
    translator_arrows_[0] = &arrow_x;
    translator_arrows_[1] = &arrow_y;
    translator_arrows_[2] = &arrow_z;
    plane_xy.SetParent(translator);
    plane_xz.SetParent(translator);
    plane_yz.SetParent(translator);
    translator_planes_[0] = &plane_yz;
    translator_planes_[1] = &plane_xz;
    translator_planes_[2] = &plane_xy;

    // ------- Create rotation gizmo -------
    SceneObject& ring_x = scene->CreateRing("_gizmo Rotator X");
    ring_x.GetTransform().Scale.Set(glm::vec3(0.62f, 0.025f, 0.62f));
    ring_x.GetTransform().Rotation.Set(glm::vec3(0.0f, 0.0f, -90.0f));
    recursive_add_map<glm::vec3>(rotation_axes, &ring_x, glm::vec3(1,0,0));
    recursive_set_material(&ring_x, red_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &ring_x, &ring_x);

    SceneObject& ring_y = scene->CreateRing("_gizmo Rotator Y");
    ring_y.GetTransform().Scale.Set(glm::vec3(0.63f, 0.025f, 0.63f));
    recursive_add_map<glm::vec3>(rotation_axes, &ring_y, glm::vec3(0,1,0));
    recursive_set_material(&ring_y, green_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &ring_y, &ring_y);

    SceneObject& ring_z = scene->CreateRing("_gizmo Rotator Z");
    ring_z.GetTransform().Rotation.Set(glm::vec3(90.0f, 0.0f, 0.0f));
    ring_z.GetTransform().Scale.Set(glm::vec3(0.625f, 0.025f, 0.625f));
    recursive_add_map<glm::vec3>(rotation_axes, &ring_z, glm::vec3(0,0,1));
    recursive_set_material(&ring_z, blue_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &ring_z, &ring_z);

    SceneObject& ring_view = scene->CreateRing("_gizmo Rotator View Plane");
    ring_view.GetTransform().Rotation.Set(glm::vec3(90.0f, 0.0f, 0.0f));
    ring_view.GetTransform().Scale.Set(glm::vec3(0.68f, 0.025f, 0.68f));
    recursive_add_map<glm::vec3>(rotation_axes, &ring_view, glm::vec3(0,0,0));
    recursive_set_material(&ring_view, yellow_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &ring_view, &ring_view);

    SceneObject& ring_view_parent = scene->CreateSceneObject("gizmo_ View Plane Rotator");
    ring_view.SetParent(ring_view_parent);

    SceneObject& rotator = scene->CreateSceneObject("_gizmo Rotator");
    ring_x.SetParent(rotator);
    ring_y.SetParent(rotator);
    ring_z.SetParent(rotator);
    rotator_disks_[0] = &ring_x;
    rotator_disks_[1] = &ring_y;
    rotator_disks_[2] = &ring_z;

    // ------- Create scale gizmo -------
    SceneObject& club_x = scene->CreateArrow("_gizmo Club X", "Cube");
    club_x.GetTransform().Rotation.Set(glm::vec3(0.0f, 0.0f, -90.0f));
    recursive_add_map<glm::vec3>(scale_axes, &club_x, glm::vec3(1,0,0));
    recursive_set_material(&club_x, red_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &club_x, &club_x);

    SceneObject& club_y = scene->CreateArrow("_gizmo Club Y", "Cube");
    recursive_add_map<glm::vec3>(scale_axes, &club_y, glm::vec3(0,1,0));
    recursive_set_material(&club_y, green_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &club_y, &club_y);

    SceneObject& club_z = scene->CreateArrow("_gizmo Club Z", "Cube");
    club_z.GetTransform().Rotation.Set(glm::vec3(90.0f, 0.0f, 0.0f));
    recursive_add_map<glm::vec3>(scale_axes, &club_z, glm::vec3(0,0,1));
    recursive_set_material(&club_z, blue_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &club_z, &club_z);

    SceneObject& scale_center = scene->CreateMesh("_gizmo Scale Center", "Cube");
    scale_center.GetTransform().Scale.Set(glm::vec3(0.04f, 0.04f, 0.04f));
    recursive_set_material(&scale_center, gray_material);
    recursive_add_map<SceneObject*>(uid2gizmo, &scale_center, &scale_center);

    SceneObject& scaler = scene->CreateSceneObject("_gizmo Scaler");
    scale_center.SetParent(scaler);
    scaler_center_ = &scale_center;
    club_x.SetParent(scaler);
    club_y.SetParent(scaler);
    club_z.SetParent(scaler);
    scaler_clubs_[0] = &club_x;
    scaler_clubs_[1] = &club_y;
    scaler_clubs_[2] = &club_z;

    // ------- Create parent gizmo -------
    SceneObject& manipulator = scene->CreateSceneObject("_gizmo Manipulator");
    SceneObject& manipulator_inverse_helper = scene->CreateSceneObject("_gizmo Manipulator Transform");
    translator.SetParent(manipulator_inverse_helper);
    rotator.SetParent(manipulator_inverse_helper);
    scaler.SetParent(manipulator_inverse_helper);
    manipulator_inverse_helper.SetParent(manipulator);
    ring_view_parent.SetParent(manipulator);
    manipulator.SetEnabled(false);
    ManipulationDone();
    manipulator_ = &manipulator;
    manipulator_transform_ = &manipulator_inverse_helper;
    translator_gizmo_ = &translator;
    rotator_gizmo_ = &rotator;
    scaler_gizmo_ = &scaler;
    viewspace_rotator_gizmo_ = &ring_view_parent;

    manipulator.SetFlag(SceneObject::INTERNAL_HIDDEN);
    scene->UnlockSignal();
}

void GizmoManager::SetTransforms(SceneObject* scene_object, SceneCamera* camera, Space space) {
    glm::mat4 mv = scene_object->GetModelMatrix();

    manipulator_->GetTransform().ForceMatrix(glm::inverse(mv));
    Transform t;
    t.SetFromMatrix(mv);
    manipulator_transform_->GetTransform().Translation.Set(t.Translation.Get());
    float d = 1;
    if (camera->IsPerspective()) {
        d = camera->GetDistance(mv*glm::vec4(0,0,0,1))*0.4f;
        manipulator_transform_->GetTransform().Scale.Set(glm::vec3(d,d,d));
    } else {
        d = camera->GetWidth()*0.25f;
        manipulator_transform_->GetTransform().Scale.Set(glm::vec3(d,d,d));
    }
    if (space == Space::Local) {
        manipulator_transform_->GetTransform().Rotation.Set(t.Rotation.Get());
    } else {
        manipulator_transform_->GetTransform().Rotation.Set(glm::vec3(0,0,0));
    }

    allow_free_translation_ = true;
    allow_free_rotation_ = true;
    for (int i = 0; i < 3; i++) {
        if (scene_object->GetTransform().Translation.GetProperty(i)->IsHidden()) {
            allow_free_translation_ = false;
            if (space != Space::Local) translator_gizmo_->SetEnabled(false, false);
            translator_arrows_[i]->SetEnabled(false, false);
        } else {
            translator_arrows_[i]->SetEnabled(true, false);
        }

        if (scene_object->GetTransform().Rotation.GetProperty(i)->IsHidden()) {
            viewspace_rotator_gizmo_->SetEnabled(false, false);
            allow_free_rotation_ = false;
            if (space != Space::Local) rotator_gizmo_->SetEnabled(false, false);
            rotator_disks_[i]->SetEnabled(false, false);
        } else {
            glm::vec4 axis(0,0,0,0);
            axis[i] = 1;
            glm::mat4 m = manipulator_transform_->GetModelMatrix();
            glm::vec3 world_space_axis = m*axis;
            world_space_axis = glm::normalize(world_space_axis);
            recursive_add_map<glm::vec3>(rotation_axes, rotator_disks_[i], world_space_axis);
        }

        if (scene_object->GetTransform().Scale.GetProperty(i)->IsHidden()) {
            allow_uniform_scale_ = false;
            if (space != Space::Local) scaler_gizmo_->SetEnabled(false, false);
            scaler_clubs_[i]->SetEnabled(false, false);
        } else {
            scaler_clubs_[i]->SetEnabled(true, false);
        }
    }

    for (int i = 0; i < 3; i++) {
        bool enabled = translator_arrows_[(i+1)%3]->IsEnabled() && translator_arrows_[(i+2)%3]->IsEnabled();
        translator_planes_[i]->SetEnabled(enabled, false);
    }

    if (allow_free_rotation_) {
        glm::vec4 camera_axis(0,0,1,0);
        Transform camera_rotation;
        camera_rotation.Rotation.Set(camera->GetRotation());
        camera_axis = camera_rotation.GetMatrix()*camera_axis;
        recursive_add_map<glm::vec3>(rotation_axes, viewspace_rotator_gizmo_, camera_axis.xyz());
        viewspace_rotator_gizmo_->GetTransform().Translation.Set(t.Translation.Get());
        viewspace_rotator_gizmo_->GetTransform().Rotation.Set(camera->GetRotation());
        viewspace_rotator_gizmo_->GetTransform().Scale.Set(glm::vec3(d,d,d));
    }
}

glm::vec3 ProjectOntoPlane(glm::vec3 p, glm::vec4 plane) {
    float d = glm::dot(p, plane.xyz()) + plane.w;
    return p - d*plane.xyz();
}

void GizmoManager::TranslationBegin(const SceneCamera* camera, glm::vec3 initial_position, uint64_t click_uid)
{
    if (uid2gizmo.find(click_uid) != uid2gizmo.end()) {
        clicked_gizmo_ = uid2gizmo[click_uid];
    }
    assert(camera != nullptr);
    SceneObject* selected_object = GetManipulatedObject();
    translator_->SetCamera(camera);

    glm::mat4 mv = manipulator_transform_->GetModelMatrix();
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(mv)));
    glm::vec4 world_space_translation = mv*glm::vec4(selected_object->GetTransform().Translation.Get(), 1);
    if (translation_planes.find(click_uid) != translation_planes.end()) {
        glm::vec4 plane = translation_planes[click_uid];
        glm::vec4 point(ProjectOntoPlane(glm::vec3(0,0,0), plane), 1);
        glm::vec3 world_space_normal = glm::normalize(normal_matrix*plane.xyz());
        glm::vec4 world_space_point = mv*point;
        glm::vec4 world_space_plane(world_space_normal, -glm::dot(world_space_point.xyz(), world_space_normal));
        translator_->TranslationBegin(initial_position, world_space_plane, world_space_translation.xyz);
        manipulating_ = true;
    } else if (translation_axes.find(click_uid) != translation_axes.end()) {
        glm::vec4 axis(translation_axes[click_uid], 0);
        glm::vec3 world_space_axis = mv*axis;
        world_space_axis = glm::normalize(world_space_axis);
        translator_->AxisTranslationBegin(initial_position, world_space_axis, world_space_translation.xyz);
        manipulating_ = true;
    } else if (selected_object->GetUID() == click_uid && allow_free_translation_) {
        translator_->TranslationBegin(initial_position, glm::vec4(0,0,0,0), world_space_translation.xyz);
        manipulating_ = true;
    }
}

void GizmoManager::ObjectTranslated(int x, int y)
{
    if (!manipulating_) return;
    SceneObject* selected_object = GetManipulatedObject();
    glm::vec3 translation;
    translator_->TranslationUpdate(x, y, translation);
    glm::mat4 world2obj = glm::inverse(selected_object->GetParentModelMatrix());
    glm::vec4 local_translation = world2obj*glm::vec4(translation, 0);
    selected_object->GetTransform().Translate(local_translation, Space::World);
}

void GizmoManager::RotationBegin(const SceneCamera *camera, glm::vec3 initial_position, uint64_t click_uid)
{
    if (uid2gizmo.find(click_uid) != uid2gizmo.end()) {
        clicked_gizmo_ = uid2gizmo[click_uid];
    }
    assert(camera != nullptr);
    SceneObject* selected_object = GetManipulatedObject();
    rotator_->SetCamera(camera);

    glm::mat4 m = manipulator_transform_->GetModelMatrix();
    glm::vec4 manipulator_center(0,0,0,1);
    if (rotation_axes.find(click_uid) != rotation_axes.end()) {
        rotator_->RotationBegin(initial_position, rotation_axes[click_uid], m*manipulator_center);
        manipulating_ = true;
    } else if (selected_object->GetUID() == click_uid && allow_free_rotation_) {
        rotator_->RotationBegin(initial_position, glm::vec3(0,0,0), m*manipulator_center);
        manipulating_ = true;
    }
}

void GizmoManager::ObjectRotated(int x, int y)
{
    if (!manipulating_) return;
    double angle;
    glm::vec3 axis;
    SceneObject* selected_object = GetManipulatedObject();
    rotator_->RotationUpdate(x, y, axis, angle);
    glm::mat4 world2obj = glm::inverse(selected_object->GetModelMatrix());
    glm::vec4 local_rotation_axis = glm::normalize(world2obj*glm::vec4(axis,0));
    selected_object->GetTransform().Rotate(local_rotation_axis, angle);
}

void GizmoManager::ScaleBegin(const SceneCamera *camera, glm::vec3 initial_position, uint64_t click_uid)
{
    if (uid2gizmo.find(click_uid) != uid2gizmo.end()) {
        clicked_gizmo_ = uid2gizmo[click_uid];
    }
    assert(camera != nullptr);
    SceneObject* selected_object = GetManipulatedObject();
    scaler_->SetCamera(camera);
    glm::mat4 mv = manipulator_transform_->GetModelMatrix();
    glm::vec3 manipulator_center = mv*glm::vec4(0,0,0,1);
    if (scale_axes.find(click_uid) != scale_axes.end()) {
        scale_axis = scale_axes[click_uid];
        glm::vec4 axis(scale_axes[click_uid], 0);
        glm::vec3 world_space_axis = mv*axis;
        world_space_axis = glm::normalize(world_space_axis);
        scaler_->AxisScaleBegin(initial_position, manipulator_center, world_space_axis, selected_object->GetTransform().Scale.Get());
        manipulating_ = true;
    } else if ((selected_object->GetUID() == click_uid || scaler_center_->GetUID() == click_uid) && allow_uniform_scale_) {
        scale_axis = glm::vec3(1,1,1);
        scaler_->ScaleBegin(initial_position, manipulator_center, selected_object->GetTransform().Scale.Get());
        manipulating_ = true;
    }
}

void GizmoManager::ObjectScaled(int x, int y)
{
    if (!manipulating_) return;
    SceneObject* selected_object = GetManipulatedObject();
    glm::vec3 scale;
    scaler_->ScaleUpdate(x, y, scale);
    for (int i = 0; i < 3; i++) {
        if (scale_axis[i] < 1) scale[i] = 1;
    }
    selected_object->GetTransform().Scale.Set(scale*scaler_->GetOriginalScale());

    for (int i = 0; i < 3; i++) {
        if (scale[i] != 1) {
            scaler_clubs_[i]->GetTransform().Scale.Set(glm::vec3(1,scale[i],1));
        }
    }
}

void GizmoManager::OnObjectDeleted(SceneObject& obj) {
    if (manipulator_->GetParent()->GetUID() == obj.GetUID()) {
        manipulator_->SetParent(scene_->GetSceneRoot());
    }
}

void GizmoManager::OnObjectSelected(uint64_t uid) {
    SceneObject* scene_object = scene_->FindSceneObject(uid);
    if (scene_object && !scene_object->IsInternal()) SelectObject(scene_object);
}

void GizmoManager::SelectObject(SceneObject* scene_object) {
    manipulator_->SetParent(*scene_object);
}

void GizmoManager::ManipulationDone() {
    clicked_gizmo_ = nullptr;
    for (int i = 0; i < 3; i++) {
        scaler_clubs_[i]->GetTransform().Scale.Set(glm::vec3(1.f,1.f,1.f));
    }
    manipulating_ = false;
}

SceneObject* GizmoManager::GetManipulatedObject() {
    return manipulator_->GetParent();
}
