/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef MATERIAL_H
#define MATERIAL_H

#include <animator.h>
#include <resource/texture.h>
#include <resource/cubemap.h>
#include <resource/shaderprogram.h>
#include <resource/asset.h>

// Material defines how some piece of geometry is rendered, and inputs the Shaders require.
// See: https://docs.unity3d.com/Manual/class-Material.html
class Material : public Asset {
public:
    ResourceProperty<ShaderProgram> Shader;
    PropertyGroup Uniforms;

    Material(const std::string& name, ShaderProgram* shader_program);
    ~Material() {
        for (auto& uf : Uniforms.GetProperties()) {
           delete Uniforms.GetProperty(uf);
        }
    }
    // TODO: Create a Copy constructor so things can instance materials and not always use the "shared" material
    //  i.e. what if two things use the same material but I just want to make one of the them black
    virtual AssetType GetType() const override { return AssetType::Material; }

    void OnShaderSet(Asset* shader);
    void OnShaderProgramChanged();

    //This is to make sure shader is loaded BEFORE uniforms
    virtual void LoadFromYAML(const YAML::Node& node) {
       assert(node.IsMap());
       Shader.LoadFromYAML(node["Shader"]);
       Uniforms.LoadFromYAML(node["Uniforms"]);
    }

    // Returns false if this material can't trace
    // If you change trace inputs, make sure they are added in Material::OnShaderSet in the cpp
    bool PrepareToTrace() {
        if (!(Shader.Get()->TraceCompatible.Get())) {
            return false;
        }

        //If there is a nullptr crash here, the material isn't getting the trace properties
        Emissive = dynamic_cast<TextureProperty*>(Uniforms.GetProperty("Emissive"))->Get();
        Specular = dynamic_cast<TextureProperty*>(Uniforms.GetProperty("Specular"))->Get();
        Diffuse = dynamic_cast<TextureProperty*>(Uniforms.GetProperty("Diffuse"))->Get();
        Transmittence = dynamic_cast<TextureProperty*>(Uniforms.GetProperty("Transmittence"))->Get();
        Shininess = dynamic_cast<DoubleProperty*>(Uniforms.GetProperty("Shininess"))->Get();
        IndexOfRefraction = dynamic_cast<DoubleProperty*>(Uniforms.GetProperty("IndexOfRefraction"))->Get();

        return true;
    }

    //These are for caching the things we will use for trace
    Texture* Emissive;
    Texture* Specular;
    Texture* Diffuse;
    Texture* Transmittence;
    double Shininess;
    double IndexOfRefraction;
};

#endif // MATERIAL_H
