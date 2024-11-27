/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "scenemanager.h"
#include <fileio.h>
#include <yaml-cpp/yaml.h>
#include "yamlextensions.h"
#include <scene/scene.h>
#include <animation/curvesampler.h>
#include <fstream>
#include <deque>
#include <string>
#include <sstream>

template<> SceneManager* Singleton<SceneManager>::_instance_ = nullptr;

SceneManager::SceneManager(ShaderFactory& shader_factory, CurveSamplerFactory& curve_factory) :
    Singleton<SceneManager>(),
    shader_factory_(&shader_factory),
    curve_factory_(&curve_factory)
{
}

void SceneManager::SaveScene(const std::string& scene_name, const std::string& filename) {
    YAML::Emitter out;
    out.SetIndent(4);
    out.SetFloatPrecision(5);
    out.SetDoublePrecision(5);

    try {
        Scene::Instance()->SetName(scene_name);
        Scene::Instance()->SaveToYAML(out);
        FileIO::WriteTextFile(filename, out.c_str());
    } catch (const std::exception& e) {
        Debug::Log.WriteLine("Could not save scene to \"" + filename + "\"", Priority::Error);
        Debug::Log.WriteLine("    " + std::string(e.what()));
    }
}

Scene* SceneManager::LoadScene(const std::string& filename) {
    loading_ = true;

    Scene* current = Scene::Instance();
    if (current) {
        delete current;
    }
    current = new Scene("Untitled Scene", *shader_factory_);

    try {
        YAML::Node scene_file = YAML::LoadFile(filename);
        current->LoadFromYAML(scene_file);
    } catch (const std::exception& e) {
        Debug::Log.WriteLine("Could not load scene \"" + filename + "\"", Priority::Error);
        Debug::Log.WriteLine("    " + std::string(e.what()));
        loading_ = false;
        return nullptr;
    }

    loading_ = false;
    return current;
}

Scene* SceneManager::NewScene(const std::string& scene_name) {
    loading_ = true;
    Scene* current = Scene::Instance();
    if (current) {
        delete current;
    }
    current = new Scene(scene_name, *shader_factory_);
    loading_ = false;
    return current;
}

