/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <properties.h>
#include <resource/asset.h>
#include <resource/cacheable.h>
#include <vectors.h>

// ShaderProgram is a special kind of program run on the GPU that determines how texture and lighting information are combined to general the pixels of the renderered object onscreen.
// See: https://docs.unity3d.com/Manual/class-Material.html
class ShaderProgram : public Asset, public Cacheable {
public:
    // List of built-in uniforms
    static const std::set<std::string> BuiltinUniforms() {
        static const std::set<std::string> uniforms = {
            // Matrices
            "model_matrix",         // Mat4
            "view_matrix",          // Mat4
            "projection_matrix",    // Mat4
            // Screen
            "screen_width",
            "screen_height",
            // Color picking
            "object_id",
            // Environment mapping
            "environment_map",
            // Directional Lights
            "dir_light_ambient",
            "dir_light_intensity",
            "dir_light_direction",
            "dir_light_shadowmap",
            // Point Lights
            "point_light_ambient",
            "point_light_intensity",
            "point_light_position",
            "point_light_atten_const",
            "point_light_atten_linear",
            "point_light_atten_quad",
            "point_light_shadowmap",
            // Spot Lights
            "spot_light_ambient",
            "spot_light_intensity",
            "spot_light_position",
            "spot_light_direction",
            "spot_light_atten_const",
            "spot_light_atten_linear",
            "spot_light_atten_quad",
            "spot_light_cutoff",
            "spot_light_falloff",
            // Area Lights
            "area_light_ambient",
            "area_light_intensity",
            "area_light_position",
            "area_light_atten_const",
            "area_light_atten_linear",
            "area_light_atten_quad",
        };
        return uniforms;
    }

    Signal0<> Changed;
    FileProperty VertexShader;
    FileProperty FragmentShader;
    FileProperty GeometryShader;
    BooleanProperty TraceCompatible;

    ShaderProgram(const std::string& name, const Cacheable* source) :
        Asset(name),
        Cacheable(source),
        VertexShader(FileType::VertexShader),
        FragmentShader(FileType::FragmentShader),
        GeometryShader(FileType::GeometryShader),
        TraceCompatible(false)
    {
        AddProperty("Vertex", &VertexShader);
        AddProperty("Fragment", &FragmentShader);
        AddProperty("Geometry", &GeometryShader);
        AddProperty("Trace Compatible", &TraceCompatible);
        shader_texts_[ShaderType::Vertex] = "";
        shader_texts_[ShaderType::Fragment] = "";
        shader_texts_[ShaderType::Geometry] = "";
    }
    ShaderProgram(const std::string& name) :
        ShaderProgram(name, nullptr)
    { }



    virtual bool IsValidShaderProgram() const = 0;
    virtual AssetType GetType() const override { return AssetType::ShaderProgram; }
    virtual std::vector<std::pair<std::string, DataType>> GetShaderInputs() const = 0;
    virtual void SetShader(const std::string& name, const std::string& source, ShaderType shader_type) = 0;

    virtual const std::string GetShaderText(ShaderType type) const { return shader_texts_.find(type)->second; }

protected:
    std::unordered_map<ShaderType, std::string, EnumClassHash> shader_texts_;
};

#endif // SHADERPROGRAM_H
