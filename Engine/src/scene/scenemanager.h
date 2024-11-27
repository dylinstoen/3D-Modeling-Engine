/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <string>
#include <memory>
#include <singleton.h>

namespace YAML {
    class Emitter;
    class Node;
}

class Scene;
class AssetManager;
class SceneObject;
class ShaderFactory;
class CurveSamplerFactory;
class Property;
class CurveSampler;
class Component;

class SceneManager;

// SceneManager handles the loading and saving of Scenes
class SceneManager : public Singleton<SceneManager> {
public:
    SceneManager(ShaderFactory& shader_factory, CurveSamplerFactory& curve_factory);
    void SaveScene(const std::string& scene_name, const std::string& filename);
    Scene* NewScene(const std::string& scene_name);
    // Loads the scene from disk. Returns nullptr if failed.
    Scene* LoadScene(const std::string& filename);

    ShaderFactory* GetShaderFactory() { return shader_factory_; }
    CurveSamplerFactory* GetCurveSamplerFactory() { return curve_factory_; }

    bool IsLoading() {
        return loading_;
    }

private:
    // Asset factories
    ShaderFactory* shader_factory_;
    CurveSamplerFactory* curve_factory_;
    bool loading_ = false;
};

#endif // SCENEMANAGER_H
