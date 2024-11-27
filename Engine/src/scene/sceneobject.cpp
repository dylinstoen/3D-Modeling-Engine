/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "sceneobject.h"
#include <scene/scene.h>
#include <scene/components/geometry.h>
#include <scene/components/robotarmprop.h>
#include <scene/components/customprop.h>

SceneObject::SceneObject(const std::string& name, int flag) :
    uid_(SceneObject::uid_counter_++),
    name_(name),
    parent_(nullptr),
    enabled_(true),
    flag_(flag)
{
    AddComponent<Transform>();
}

SceneObject::~SceneObject() {
    // Clear all signals
    ComponentAdded.Clear();
    ComponentRemoved.Clear();
    LightSourceAdded.Clear();
    NameChanged.Clear();
    EnabledChanged.Clear();
    ParentChanged.Clear();
}

Component* SceneObject::AddComponent(std::string classname) {
    Component* component = Component::Create(classname);
    AddComponent(component);
    return component;
}

void SceneObject::AddComponent(Component* component) {
    // Let whomever know that this object is now a light source
    if (dynamic_cast<PointLight*>(component) || dynamic_cast<DirectionalLight*>(component) || dynamic_cast<SpotLight*>(component)) LightSourceAdded.Emit(*this);
    // Let whomever know that this object is now a particle system
    if (dynamic_cast<ParticleSystem*>(component)) ParticleSystemAdded.Emit(*this);
    if (dynamic_cast<ConnectedParticleSystem*>(component)) ConnectedParticleSystemAdded.Emit(*this);
    assert(components_.find(component->GetBaseType()) == components_.end());
    components_[component->GetBaseType()] = component;
    ComponentAdded.Emit(*component);
}


void SceneObject::SetParticleGeom(std::string name) {
    if (GetComponent<Sphere>() != nullptr) {
        RemoveComponent<Sphere>();
    } else if (GetComponent<Cylinder>() != nullptr) {
        RemoveComponent<Cylinder>();
    } else if (GetComponent<Plane>() != nullptr) {
        RemoveComponent<Plane>();
    } else if (GetComponent<TriangleMesh>() != nullptr) {
        RemoveComponent<TriangleMesh>();
    } else {
        qDebug("Invalid particle geometry state!");
        return;
    }
    if (name == "Sphere") {
        AddComponent<Sphere>();
    } else if (name == "Cylinder") {
        AddComponent<Cylinder>();
    } else if (name == "Plane") {
        AddComponent<Plane>();
    } else if (name == "Mesh") {
        AddComponent<TriangleMesh>();
        AssetManager& asset_manager = *(AssetManager::Instance());
        auto mesh = asset_manager.GetMesh("Cube");
        assert(mesh != nullptr);
        GetComponent<TriangleMesh>()->MeshFilter.Set(mesh);
    }
}

void SceneObject::SaveToYAML(YAML::Emitter &out) const
{
    out << YAML::BeginMap;
    out << YAML::Key << "Name" << YAML::Value << name_;
    out << YAML::Key << "Enabled" << YAML::Value << enabled_;

    out << YAML::Key << "Components" << YAML::Value << YAML::BeginMap;
    for(auto it = components_.begin(); it!=components_.end(); it++) {
        out << YAML::Key << it->second->GetTypeName() << YAML::Value;
        it->second->SaveToYAML(out);
    }
    out << YAML::EndMap;

    out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
    for (auto it=children_.begin(); it!=children_.end(); it++) {
        if (!it->second->IsInternal()) {
            out << YAML::Value;
            it->second->SaveToYAML(out);
        }
    }
    out << YAML::EndSeq;

    out << YAML::EndMap;
}

void SceneObject::LoadFromYAML(const YAML::Node &node)
{
    assert(node.IsMap());
    assert(node["Name"]);
    SetName(node["Name"].as<std::string>());
    SetEnabled(node["Enabled"] ? node["Enabled"].as<bool>() : true);

    assert(node["Components"] && node["Components"].IsMap());
    for(auto it=node["Components"].begin(); it!=node["Components"].end(); it++)
    {
        Component* added = it->first.as<std::string>()=="Transform" ? &GetTransform() : AddComponent(it->first.as<std::string>());

        added->LoadFromYAML(it->second);

        std::string comp_name = it->first.as<std::string>();
        if (comp_name == "RobotArmProp")
            dynamic_cast<RobotArmProp*>(added)->SetRoot(this);
        else if (comp_name == "CustomProp")
            dynamic_cast<CustomProp*>(added)->SetRoot(this);
    }

    assert(node["Children"] && node["Children"].IsSequence());
    for(auto it=node["Children"].begin(); it!=node["Children"].end(); it++) {
        SceneObject* added = &(Scene::Instance()->CreateSceneObject("temp"));
        added->LoadFromYAML(*it);
        added->SetParent(*this);
    }
}

uint64_t SceneObject::uid_counter_ = 0;
