/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SCENE_H
#define SCENE_H

#include <animator.h>
#include <scene/sceneobject.h>
#include <resource/assetmanager.h>
#include <scene/scenecamera.h>
#include <components.h>
#include <serializable.h>
#include <singleton.h>

class Scene;

// Scene owns all the Scene Objects
class Scene : public Serializable, public Singleton<Scene> {
public:
    Scene(std::string name, ShaderFactory &shader_factory);

    void SetName(const std::string& name) { name_ = name; NameChanged.Emit(name_); }
    std::string GetName() { return name_; }
    SceneObject& GetSceneRoot();
    SceneCamera& GetSceneCamera() { return scene_camera_; }
    AssetManager& GetAssetManager() { return asset_manager_; }

    // Returns the first render cam or makes it
    SceneObject* GetOrCreateRenderCam();
    std::vector<SceneObject*> GetRenderCams();

    // Returns a list of SceneObjects with Colliders and their ModelMatrices
    // Updated whenever UpdatePrepass is called.
    std::vector<std::pair<SceneObject*, glm::mat4>> GetColliders();

    // Returns a list of SceneObjects with Lights and their ModelMatrices
    // Updated whenever RenderPrepass is called.
    std::vector<std::pair<SceneObject*, glm::mat4>> GetLights();

    // Returns a list of SceneObjects with EnvironmentMaps and their ModelMatrices
    // Updated whenever RenderPrepass is called.
    std::vector<std::pair<SceneObject*, glm::mat4>> GetEnvMaps();

    // Animation Properties
    unsigned int GetAnimationLength() const { return animation_length_; }
    void SetAnimationLength(unsigned int animation_length) { animation_length_ = animation_length; }
    unsigned int GetFPS() const { return fps_; }
    void SetFPS(unsigned int fps) { fps_ = fps; }
    void SetRealtime(bool set) { realtime_ = set; }

    // Animation Functions
    void Start();
    void Stop();
    void Reset();
    void RenderPrepass();
    void Update(float t, float delta_t);

    // SceneObject Manipulation
    SceneObject* FindSceneObject(uint64_t UID);
    SceneObject& CreateSceneObject(const std::string& name, int flag=SceneObject::NOT_INTERNAL);
    void ReparentSceneObject(uint64_t object_id, uint64_t parent_id);
    void DeleteSceneObject(uint64_t uid);
    SceneObject* DuplicateSceneObject(uint64_t uid);

    // SceneObject Creation
    SceneObject& CreateMesh(const std::string& name, const std::string& mesh_name);
    SceneObject& CreateSurfaceOfRevolution(const std::string& name);
    SceneObject& CreatePlane(const std::string& name);
    SceneObject& CreateArrow(const std::string& name, const std::string& arrowhead_shape = "Pyramid");
    SceneObject& CreateSphere(const std::string& name);
    SceneObject& CreateCylinder(const std::string& name);
    SceneObject& CreateRing(const std::string& name);
    SceneObject& CreateCone(const std::string& name);
    SceneObject& CreatePointLight(const std::string& name);
    SceneObject& CreateDirectionalLight(const std::string& name);
    SceneObject& CreateSpotLight(const std::string& name);
    SceneObject& CreateAreaLight(const std::string& name);
    SceneObject& CreateParticleSystem(const std::string& name);
    SceneObject& CreateConnectedParticleSystem(const std::string& name);
    SceneObject& CreateCamera(const std::string& name);
    SceneObject& CreateSphereCollider(const std::string& name);
    SceneObject& CreatePlaneCollider(const std::string& name);
    SceneObject& CreateCylinderCollider(const std::string& name);

    void LockSignal() { signal_lock_ = true; }
    void UnlockSignal() {
        for (size_t i = 0; i < signalqueue.size(); i++) SceneObjectCreated.Emit(*signalqueue[i]);
        signalqueue.clear();
        signal_lock_ = false;
    }

    // Signals
    Signal1<const std::string&> NameChanged;
    Signal1<SceneObject&> CameraCreated;
    Signal1<SceneObject&> SceneObjectCreated;
    Signal1<SceneObject&> SceneObjectDeleted;
    Signal1<SceneObject&> SceneObjectDuplicated;
    Signal1<SceneObject&> SceneObjectHideToggled;

protected:
    std::string name_;
    AssetManager asset_manager_;
    std::unique_ptr<SceneObject> scene_root_;
    std::unordered_map<uint64_t, std::unique_ptr<SceneObject>> scene_objects_;
    std::unordered_map<uint64_t, SceneObject*> light_sources_;
    std::unordered_map<uint64_t, SceneObject*> particle_systems_;
    SceneCamera scene_camera_;

    std::vector<std::pair<SceneObject*, glm::mat4>> colliders_;
    void UpdatePrepass(SceneObject& node, glm::mat4 model_matrix);
    void SetAnimationTime(float t, ObjectWithProperties* o);
    std::vector<std::pair<SceneObject*, glm::mat4>> lights_;
    std::vector<std::pair<SceneObject*, glm::mat4>> envmaps_;
    void RenderPrepass(SceneObject& node, glm::mat4 model_matrix);
    SceneObject* render_cam_;

    // Animation Properties
    unsigned int animation_length_;
    unsigned int fps_;
    bool realtime_;

    bool signal_lock_;
    // Unsignalled object creation queue
    std::vector<SceneObject*> signalqueue;

    void RegisterLightSource(SceneObject& light) { light_sources_[light.GetUID()] = &light; }
    void RegisterParticleSystem(SceneObject& ps) { particle_systems_[ps.GetUID()] = &ps; }

    // Serializable interface
public:
    void SaveToYAML(YAML::Emitter &out) const;
    void LoadFromYAML(const YAML::Node &node);
};

#endif // SCENE_H
