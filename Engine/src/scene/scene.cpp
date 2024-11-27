/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <animator.h>
#include <scene/scene.h>
#include <resource/assetmanager.h>
#include <scene/components/geometry.h>

template<> Scene* Singleton<Scene>::_instance_ = nullptr;

Scene::Scene(std::string name, ShaderFactory& shader_factory) :
    Singleton<Scene>(),
    name_(name),
    asset_manager_(shader_factory),
    scene_root_(std::make_unique<SceneObject>("Root")),
    render_cam_(nullptr),
    animation_length_(0),
    fps_(0),
    signal_lock_(false)
{
}

SceneObject* Scene::FindSceneObject(uint64_t UID) {
    if (scene_objects_.count(UID) > 0)
        return scene_objects_[UID].get();
    return nullptr;
}

// Only used for SceneObjects that are not the root
SceneObject& Scene::CreateSceneObject(const std::string& name, int flag) {
    std::unique_ptr<SceneObject> node = std::make_unique<SceneObject>(name, flag);
    node->LightSourceAdded.Connect(this, &Scene::RegisterLightSource);
    node->ParticleSystemAdded.Connect(this, &Scene::RegisterParticleSystem);
    node->ConnectedParticleSystemAdded.Connect(this, &Scene::RegisterParticleSystem);
    uint64_t uid = node->GetUID();
    assert(scene_objects_.count(uid) == 0); // It's not a "unique" id if it exists already
    // Set the parent to be the root
    node->SetParent(GetSceneRoot());

    scene_objects_[uid] = std::move(node);

    if (signal_lock_) {
        signalqueue.push_back(scene_objects_[uid].get());
    } else {
        SceneObjectCreated.Emit(*scene_objects_[uid]);
    }

    return *scene_objects_[uid];
}

void Scene::DeleteSceneObject(uint64_t uid) {
    assert(scene_objects_.count(uid) > 0); // It can't have been removed already!
    SceneObject* obj = scene_objects_[uid].get();
    if (obj->IsInternal()) return;
    // Need to recursively remove all children too
    for (auto& child : obj->GetChildren()) {
        DeleteSceneObject(child->GetUID());
    }

    // Remove references to this object
    if (light_sources_.count(uid) > 0) light_sources_.erase(uid);
    obj->GetParent()->RemoveChild(uid);

    // Notify the UI of the deletion first, before the pointer is invalidated
    SceneObjectDeleted.Emit(*obj);

    // All pointers to the scene object are invalidated after this
    scene_objects_.erase(uid);

    if (obj == render_cam_) {
        render_cam_ = nullptr;
        GetOrCreateRenderCam();
    }
}

SceneObject* Scene::DuplicateSceneObject(uint64_t uid) {
    assert(scene_objects_.count(uid) > 0); // It can't have been removed already!
    SceneObject* obj = scene_objects_[uid].get();
    assert(!obj->IsInternal());

    std::string newObjName = obj->GetName() + " 2";

    SceneObject& newObj = CreateSceneObject(newObjName);
    newObj.SetParent(*(obj->GetParent()));

    YAML::Emitter objdata;
    obj->SaveToYAML(objdata);
    YAML::Node root = YAML::Load(objdata.c_str()); //TODO: Is this a memory leak?
    newObj.LoadFromYAML(root);
    newObj.SetName(newObjName);

    return &newObj;
}

void Scene::RenderPrepass() {
    lights_.clear();
    envmaps_.clear();
    RenderPrepass(GetSceneRoot(), glm::mat4());
}

void Scene::Start() {
    // Start all particle systems
    for (auto& kv : particle_systems_) {
        // If we can't find the scene object, it's been deleted
        // TODO: Remove it
        if (FindSceneObject(kv.first) == nullptr) continue;
        if (ParticleSystem* ps = kv.second->GetComponent<ParticleSystem>()) {
            ps->StartSimulation();
        }
        if (ConnectedParticleSystem* ps2 = kv.second->GetComponent<ConnectedParticleSystem>()) {
            ps2->StartSimulation();
        }
    }
}

void Scene::Stop() {
    // Stop all particle systems
    for (auto& kv : particle_systems_) {
        // If we can't find the scene object, it's been deleted
        // TODO: Remove it
        if (FindSceneObject(kv.first) == nullptr) continue;
        if (ParticleSystem* ps = kv.second->GetComponent<ParticleSystem>()) {
             ps->StopSimulation();
        }
        if (ConnectedParticleSystem* ps2 = kv.second->GetComponent<ConnectedParticleSystem>()) {
            ps2->StopSimulation();
        }
    }
}

void Scene::Reset() {
    // Reset all particle systems
    for (auto& kv : particle_systems_) {
        // If we can't find the scene object, it's been deleted
        // TODO: Remove it
        if (FindSceneObject(kv.first) == nullptr) continue;
        if (ParticleSystem* ps = kv.second->GetComponent<ParticleSystem>()) {
             ps->ResetSimulation();
        }
        if (ConnectedParticleSystem* ps2 = kv.second->GetComponent<ConnectedParticleSystem>()) {
            ps2->ResetSimulation();
        }
    }
}

void Scene::UpdatePrepass(SceneObject& node, glm::mat4 model_matrix) {
    model_matrix = model_matrix * node.GetTransform().GetMatrix();

    // Update Particle Simulations
    if (ParticleSystem* ps = node.GetComponent<ParticleSystem>()) {
        // Store the model matrix in the Particle System so it can use World coordinates
        ps->UpdateModelMatrix(model_matrix);
    }
    if (ConnectedParticleSystem* ps2 = node.GetComponent<ConnectedParticleSystem>()) {
        // Store the model matrix in the Particle System so it can use World coordinates
        ps2->UpdateModelMatrix(model_matrix);
    }

    // Save the colliders
    SphereCollider* sphere = node.GetComponent<SphereCollider>();
    if (sphere != nullptr) colliders_.push_back(std::make_pair(&node, model_matrix));
    PlaneCollider* plane = node.GetComponent<PlaneCollider>();
    if (plane != nullptr) colliders_.push_back(std::make_pair(&node, model_matrix));
    CylinderCollider* cylinder = node.GetComponent<CylinderCollider>();
    if (cylinder != nullptr) colliders_.push_back(std::make_pair(&node, model_matrix));

    // Recurse into children
    auto children = node.GetChildren();
    for (auto& child : children) {
        if (child->IsEnabled()) UpdatePrepass(*child, model_matrix);
    }
}

void Scene::RenderPrepass(SceneObject& node, glm::mat4 model_matrix) {
    model_matrix = model_matrix * node.GetTransform().GetMatrix();

    // Save the lights
    Light* light = node.GetComponent<Light>();
    if (light != nullptr) lights_.push_back(std::make_pair(&node, model_matrix));

    // Save the envmaps
    EnvironmentMap* envmap = node.GetComponent<EnvironmentMap>();
    if (envmap != nullptr) envmaps_.push_back(std::make_pair(&node, model_matrix));

    // Recurse into children
    auto children = node.GetChildren();
    for (auto& child : children) {
        if (child->IsEnabled()) RenderPrepass(*child, model_matrix);
    }
}

void Scene::Update(float t, float delta_t) {

    colliders_.clear();
    UpdatePrepass(GetSceneRoot(), glm::mat4());

    // Find all animatable properties and particle systems
    for (auto& kv : scene_objects_) {
        // Update Particle Simulation
        if (delta_t > 0) {
            if (ParticleSystem* ps = kv.second->GetComponent<ParticleSystem>()) {
                if (realtime_)
                    ps->UpdateSimulation(delta_t, colliders_);
                else
                    ps->UpdateSimulation(1.0f / fps_, colliders_);
            }
            if (ConnectedParticleSystem* ps2 = kv.second->GetComponent<ConnectedParticleSystem>()) {
                if (realtime_)
                    ps2->UpdateSimulation(delta_t, colliders_);
                else
                    ps2->UpdateSimulation(1.0f / fps_, colliders_);
            }
        }

        // Find animatable properties
        for (auto& component : kv.second->GetComponents()) {
                SetAnimationTime(t, component);
        }
    }
}

void Scene::SetAnimationTime(float t, ObjectWithProperties* o) {
    for(auto pname : o->GetProperties()) {
        Property* p = o->GetProperty(pname);
        if (auto owp = dynamic_cast<ObjectWithProperties*>(p)) {
            SetAnimationTime(t, owp);
        }
        if (auto doub = dynamic_cast<DoubleProperty*>(p)) {
            doub->SetAnimationTime(t);
        }
    }
}

SceneObject &Scene::GetSceneRoot() {
    // SceneRoot is meant to be invisible, therefore it is not referencable by UID.
    // i.e. you can't delete or duplicate the root. The exception is setting a parent to the root.
    return *scene_root_;
}

SceneObject* Scene::GetOrCreateRenderCam() {
    if (render_cam_ == nullptr) {
        for (auto& kv : scene_objects_) {
            if (kv.second->GetComponent<Camera>()) {
                render_cam_ = kv.second.get();
                break;
            }
        }
    }
    if (render_cam_ == nullptr) {
        render_cam_ = &CreateCamera("Render Camera");
        render_cam_->GetTransform().Translation.Set(glm::vec3(0,0,2.5));
    }
    return render_cam_;
}

std::vector<SceneObject*> Scene::GetRenderCams() {
    std::vector<SceneObject*> cams;
    for (auto& kv : scene_objects_) {
        if (kv.second->GetComponent<Camera>()) {
            cams.push_back(kv.second.get());
        }
    }
    return cams;
}

std::vector<std::pair<SceneObject*, glm::mat4>> Scene::GetColliders() {
    return std::vector<std::pair<SceneObject*, glm::mat4>>(colliders_);
}

std::vector<std::pair<SceneObject*, glm::mat4>> Scene::GetLights() {
    return std::vector<std::pair<SceneObject*, glm::mat4>>(lights_);
}

std::vector<std::pair<SceneObject*, glm::mat4>> Scene::GetEnvMaps() {
    return std::vector<std::pair<SceneObject*, glm::mat4>>(envmaps_);
}

void Scene::ReparentSceneObject(uint64_t object_id, uint64_t parent_id) {
    if (scene_objects_.count(object_id) < 1) {
        // Output some message
        return;
    }
    glm::mat4 model = scene_objects_[object_id]->GetModelMatrix();
    if (parent_id == scene_root_->GetUID()) {
        scene_objects_[object_id]->SetParent(*scene_root_);
        scene_objects_[object_id]->GetTransform().SetFromMatrix(model);
    }
    else if (scene_objects_.count(parent_id) > 0) {
        glm::mat4 parent_model = scene_objects_[parent_id]->GetModelMatrix();
        scene_objects_[object_id]->SetParent(*scene_objects_[parent_id]);
        scene_objects_[object_id]->GetTransform().SetFromMatrix(glm::inverse(parent_model)*model);
    }
}

SceneObject& Scene::CreateMesh(const std::string& name, const std::string& mesh_name) {
    SceneObject& obj = CreateSceneObject(name);
    obj.AddComponent<TriangleMesh>();
    auto mesh = asset_manager_.GetMesh(mesh_name);
    assert(mesh != nullptr);
    obj.GetComponent<TriangleMesh>()->MeshFilter.Set(mesh);
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    obj.GetComponent<Geometry>()->RenderMaterial.Set(material);
    return obj;
}

SceneObject& Scene::CreateSurfaceOfRevolution(const std::string& name) {
    SceneObject& surface = CreateSceneObject(name);
    surface.AddComponent<SurfaceOfRevolution>();
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    surface.GetComponent<Geometry>()->RenderMaterial.Set(material);
    return surface;
}

SceneObject& Scene::CreatePlane(const std::string& name) {
    SceneObject& plane = CreateSceneObject(name);
    plane.AddComponent<Plane>();
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    plane.GetComponent<Geometry>()->RenderMaterial.Set(material);
    return plane;
}

SceneObject& Scene::CreateArrow(const std::string& name, const std::string& arrowhead_shape) {
    SceneObject& arrowhead = CreateMesh(name + " Arrowhead", arrowhead_shape);
    auto arrow_mesh = asset_manager_.GetMesh(arrowhead_shape);
    arrowhead.GetComponent<TriangleMesh>()->MeshFilter.Set(arrow_mesh);
    arrowhead.GetTransform().Scale.Set(glm::vec3(0.07f, 0.07f, 0.07f));
    arrowhead.GetTransform().Translation.Set(glm::vec3(0.0f, 0.3f, 0.0f));
    SceneObject& arrowshaft = CreateMesh(name + " Arrowshaft", "Cube");
    arrowshaft.GetTransform().Scale.Set(glm::vec3(0.01f, 0.28f, 0.01f));
    arrowshaft.GetTransform().Translation.Set(glm::vec3(0.0f, 0.14f, 0.0f));
    SceneObject& arrow = CreateSceneObject(name);
    arrowhead.SetParent(arrow);
    arrowshaft.SetParent(arrow);

    Material& material = asset_manager_.GetDefaultMaterial();
    arrowhead.GetComponent<Geometry>()->RenderMaterial.Set(&material);
    arrowshaft.GetComponent<Geometry>()->RenderMaterial.Set(&material);
    return arrow;
}

SceneObject& Scene::CreateSphere(const std::string& name) {
    SceneObject& sphere = CreateSceneObject(name);
    sphere.AddComponent<Sphere>();
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    sphere.GetComponent<Geometry>()->RenderMaterial.Set(material);
    return sphere;
}

SceneObject& Scene::CreateCylinder(const std::string& name) {
    SceneObject& cylinder = CreateSceneObject(name);
    cylinder.AddComponent<Cylinder>();
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    cylinder.GetComponent<Geometry>()->RenderMaterial.Set(material);
    return cylinder;
}

SceneObject &Scene::CreateRing(const std::string &name) {
    SceneObject& ring = CreateSceneObject(name);
    ring.AddComponent<Ring>();
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    ring.GetComponent<Geometry>()->RenderMaterial.Set(material);
    return ring;
}

SceneObject& Scene::CreateCone(const std::string& name) {
    SceneObject& cone = CreateSceneObject(name);
    cone.AddComponent<Cone>();
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    cone.GetComponent<Geometry>()->RenderMaterial.Set(material);
    return cone;
}

SceneObject& Scene::CreatePointLight(const std::string& name) {
    SceneObject& light = CreateSceneObject(name);
    light.AddComponent<PointLight>();
    EnvironmentMap& envmap = light.AddComponent<EnvironmentMap>();
    envmap.RenderMaterial.Set(asset_manager_.GetMaterial("Depth Map Material"));
    return light;
}

SceneObject& Scene::CreateDirectionalLight(const std::string& name) {
    SceneObject& light = CreateSceneObject(name);
    light.AddComponent<DirectionalLight>();
    return light;
}

SceneObject& Scene::CreateSpotLight(const std::string& name) {
    SceneObject& light = CreateSceneObject(name);
    light.AddComponent<SpotLight>();
    return light;
}

SceneObject& Scene::CreateAreaLight(const std::string& name) {
    SceneObject& light = CreateSceneObject(name);
    light.AddComponent<AreaLight>();
    return light;
}

SceneObject& Scene::CreateParticleSystem(const std::string &name) {
    SceneObject& ps = CreateSceneObject(name);
    ps.AddComponent<Sphere>();
    ps.AddComponent<ParticleSystem>();
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    ParticleSystem *sys = ps.GetComponent<ParticleSystem>();
    sys->ParticleMaterial.Set(material);
    sys->GeomChanged.Connect(&ps, &SceneObject::SetParticleGeom);
    return ps;
}

SceneObject& Scene::CreateConnectedParticleSystem(const std::string &name) {
    SceneObject& ps2 = CreateSceneObject(name);
    ps2.AddComponent<Sphere>();
    ps2.AddComponent<ConnectedParticleSystem>();
    auto material = asset_manager_.GetMaterial("Blinn-Phong Material");
    assert(material != nullptr);
    ConnectedParticleSystem *sys = ps2.GetComponent<ConnectedParticleSystem>();
    sys->ParticleMaterial.Set(material);
    return ps2;
}

SceneObject& Scene::CreateCamera(const std::string& name) {
    SceneObject& camera = CreateSceneObject(name);
    camera.AddComponent<Camera>();
    CameraCreated.Emit(camera);
    return camera;
}

SceneObject& Scene::CreateSphereCollider(const std::string& name) {
    SceneObject& collider = CreateSceneObject(name);
    collider.AddComponent<SphereCollider>();
    return collider;
}

SceneObject& Scene::CreatePlaneCollider(const std::string& name) {
    SceneObject& collider = CreateSceneObject(name);
    collider.AddComponent<PlaneCollider>();
    return collider;
}

SceneObject& Scene::CreateCylinderCollider(const std::string& name) {
    SceneObject& collider = CreateSceneObject(name);
    collider.AddComponent<CylinderCollider>();
    return collider;
}

void Scene::SaveToYAML(YAML::Emitter &out) const
{
    out << YAML::BeginMap;

    out << YAML::Key << "Name" << YAML::Value << name_;
    out << YAML::Key << "Animation Length" << YAML::Value << GetAnimationLength();
    out << YAML::Key << "Animation FPS" << YAML::Value << GetFPS();

    // Assets
    const AssetManager& assets = asset_manager_;

    out << YAML::Key << "Textures" << YAML::Value << YAML::BeginMap;
    for (auto& texture : assets.GetTextures()) {
        if (texture->IsInternal()) continue;
        out << YAML::Key << texture->GetName() << YAML::Value;
        texture->SaveToYAML(out);
    }
    out << YAML::EndMap;

    out << YAML::Key << "Cubemaps" << YAML::Value << YAML::BeginMap;
    for (auto& cubemap : assets.GetCubemaps()) {
        if (cubemap->IsInternal()) continue;
        out << YAML::Key << cubemap->GetName() << YAML::Value;
        cubemap->SaveToYAML(out);
    }
    out << YAML::EndMap;

    out << YAML::Key << "Meshes" << YAML::Value << YAML::BeginMap;
    for (auto& mesh : assets.GetMeshes()) {
        if (mesh->IsInternal()) continue;
        out << YAML::Key << mesh->GetName() << YAML::Value;
        mesh->SaveToYAML(out);
    }
    out << YAML::EndMap;

    out << YAML::Key << "Materials" << YAML::Value << YAML::BeginMap;
    for (auto& material : assets.GetMaterials()) {
        if (material->IsInternal()) continue;
        out << YAML::Key << material->GetName() << YAML::Value;
        material->SaveToYAML(out);
    }
    out << YAML::EndMap;

    out << YAML::Key << "ShaderPrograms" << YAML::Value << YAML::BeginMap;
    for (auto& shader_program : assets.GetShaderPrograms()) {
        if (shader_program->IsInternal()) continue;
        out << YAML::Key << shader_program->GetName() << YAML::Value;
        shader_program->SaveToYAML(out);
    }
    out << YAML::EndMap;

    // Scene Graph
    out << YAML::Key << "Root" << YAML::Value;
    scene_root_->SaveToYAML(out);

    out << YAML::EndMap;
}

void Scene::LoadFromYAML(const YAML::Node &node)
{
    SetName(node["Name"].as<std::string>());
    SetAnimationLength(node["Animation Length"].as<unsigned int>());
    SetFPS(node["Animation FPS"].as<unsigned int>());

    YAML::Node assetnode = node["Textures"];
    for (auto it = assetnode.begin(); it != assetnode.end(); it++) {
        asset_manager_.LoadTexture(it->first.as<std::string>(), it->second["Path"].as<std::string>());
    }

    assetnode = node["Cubemaps"];
    for (auto it = assetnode.begin(); it != assetnode.end(); it++) {
        asset_manager_.LoadCubemap(it->first.as<std::string>(), it->second["Path"].as<std::string>());
    }

    assetnode = node["Meshes"];
    for (auto it = assetnode.begin(); it != assetnode.end(); it++) {
        std::string name = it->first.as<std::string>();
        asset_manager_.LoadMesh(name, it->second["Path"].as<std::string>());
    }

    assetnode = node["ShaderPrograms"];
    for (auto it = assetnode.begin(); it != assetnode.end(); it++) {
        ShaderProgram* shader_program = asset_manager_.CreateShaderProgram(it->first.as<std::string>(), false);
        if (shader_program==nullptr) {
            shader_program=asset_manager_.GetShaderProgram(it->first.as<std::string>());
        }
        shader_program->LoadFromYAML(it->second);
    }

    assetnode = node["Materials"];
    for (auto it = assetnode.begin(); it != assetnode.end(); it++) {
        Material* material = asset_manager_.CreateMaterial(it->first.as<std::string>(), false);
        if (material==nullptr) {
            material = asset_manager_.GetMaterial(it->first.as<std::string>());
        }
        material->LoadFromYAML(it->second);
    }

    std::vector<uint64_t> idz;
    for (auto it : scene_root_->GetChildren()) {
        idz.push_back(it->GetUID());
    }
    for (auto it : idz) {
        DeleteSceneObject(it);
    }

    //Prevent this from rendering while loading because
    //creating curves does that
    scene_root_->SetEnabled(false);
    scene_root_->LoadFromYAML(node["Root"]);
    scene_root_->SetEnabled(true);
}
