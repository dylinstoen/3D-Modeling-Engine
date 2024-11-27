/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <resource/material.h>

Material::Material(const std::string &name, ShaderProgram* shader_program) :
    Asset(name),
    Shader(AssetType::ShaderProgram, shader_program),
    Uniforms()
{
    AddProperty("Shader", &Shader);
    AddProperty("Uniforms", &Uniforms);

    Shader.ValueSet.Connect(this, &Material::OnShaderSet);
}

void Material::OnShaderSet(Asset *asset) {
    std::map<std::string, Property*> old_properties;

    for(std::string name : Uniforms.GetProperties()) {
        old_properties[name] = Uniforms.GetProperty(name);
    }

    Uniforms.ClearProperties();

    if (asset != nullptr) {
        assert(asset->GetType() == AssetType::ShaderProgram);
        ShaderProgram* shader = dynamic_cast<ShaderProgram*>(asset);
        // Make sure the shader program is valid first
        shader->Changed.Connect(this, &Material::OnShaderProgramChanged); // TODO: Need to disconnect from previous?

        // Create a GUI Control for each uniform
        std::vector<std::pair<std::string, DataType>> uniforms = shader->GetShaderInputs();

        // Add inputs for trace if they are not here
        if (shader->TraceCompatible.Get()) {
            for (std::string textureprop : {"Emissive", "Specular", "Diffuse", "Transmittence"}) {
                bool found=false;
                for (std::pair<std::string, DataType> cur : uniforms) {
                    if (cur.first == textureprop) {
                        found=true;
                        if (cur.second != DataType::Texture2D) {
                            qDebug() << textureprop.c_str() << " uniform must be a texture";
                            throw std::exception();
                        }
                    }
                }
                if (!found) {
                    uniforms.push_back({textureprop, DataType::Texture2D});
                }
            }
            for (std::string doubleprop : {"Shininess", "IndexOfRefraction"}) {
                bool found=false;
                for (std::pair<std::string, DataType> cur : uniforms) {
                    if (cur.first == doubleprop) {
                        found=true;
                        if (cur.second != DataType::Float && cur.second != DataType::Double) {
                            qDebug() << doubleprop.c_str() << " uniform must be a float/double";
                            throw std::exception();
                        }
                    }
                }
                if (!found) {
                    uniforms.push_back({doubleprop, DataType::Double});
                }
            }
        }

        std::set<std::string> builtin_uniforms = ShaderProgram::BuiltinUniforms();

        for (auto& uniform : uniforms) {
            std::string name = uniform.first;
            if (builtin_uniforms.find(name) != builtin_uniforms.end()) {
                continue;
            }

            DataType type = uniform.second;
            Property* property = nullptr;
            Property* old_property = nullptr;
            if (old_properties.find(name) != old_properties.end()) {
                old_property = old_properties.at(name);
            }
            // TODO: refactor
            switch(type) {
                case DataType::Float:
                case DataType::Double: {
                    property = new DoubleProperty();
                    DoubleProperty* existing_prop = dynamic_cast<DoubleProperty*>(old_property);
                    if (existing_prop) dynamic_cast<DoubleProperty*>(property)->Set(existing_prop->Get());
                    if (dynamic_cast<DoubleProperty*>(property)->Get()<=0.0 && (name=="IndexOfRefraction")) { dynamic_cast<DoubleProperty*>(property)->Set(1.0); }
                    if (dynamic_cast<DoubleProperty*>(property)->Get()<=0.0 && (name=="Shininess")) { dynamic_cast<DoubleProperty*>(property)->Set(20.0); }
                    break; }
                case DataType::Float3:
                case DataType::Double3: {
                    property = new Vec3Property();
                    Vec3Property* existing_prop = dynamic_cast<Vec3Property*>(old_property);
                    if (existing_prop) dynamic_cast<Vec3Property*>(property)->Set(existing_prop->Get());
                    break; }
                case DataType::FloatMat4x4:
                case DataType::DoubleMat4x4: {
                    property = new Mat4Property();
                    Mat4Property* existing_prop = dynamic_cast<Mat4Property*>(old_property);
                    if (existing_prop) dynamic_cast<Mat4Property*>(property)->Set(existing_prop->Get());
                    break; }
                case DataType::Int: {
                    property = new IntProperty();
                    IntProperty* existing_prop = dynamic_cast<IntProperty*>(old_property);
                    if (existing_prop) dynamic_cast<IntProperty*>(property)->Set(existing_prop->Get());
                    break; }
                case DataType::Bool: {
                    property = new BooleanProperty();
                    BooleanProperty* existing_prop = dynamic_cast<BooleanProperty*>(old_property);
                    if (existing_prop) dynamic_cast<BooleanProperty*>(property)->Set(existing_prop->Get());
                    break; }
                case DataType::Texture2D: {
                    property = new TextureProperty((name=="Transmittence" || name=="Emissive") ? glm::vec3(0,0,0) : glm::vec3(0.5,0.5,0.5));
                    TextureProperty* existing_prop = dynamic_cast<TextureProperty*>(old_property);
                    if (existing_prop) {
                        if (!existing_prop->UseTexture.Get()) {
                            dynamic_cast<TextureProperty*>(property)->SetColor(existing_prop->SolidColor.GetRGB());
                        } else {
                            dynamic_cast<TextureProperty*>(property)->Set(existing_prop->Get());
                        }
                    }
                    break; }
                case DataType::ColorRGB:
                case DataType::ColorRGBA: {
                    property = new ColorProperty(type==DataType::ColorRGBA);
                    ColorProperty* existing_prop = dynamic_cast<ColorProperty*>(old_property);
                    if (existing_prop) dynamic_cast<ColorProperty*>(property)->Set(existing_prop->Get());
                    break; }
                case DataType::Cubemap: {
                    property = new ResourceProperty<Cubemap>(AssetType::Cubemap);
                    ResourceProperty<Cubemap>* existing_prop = dynamic_cast<ResourceProperty<Cubemap>*>(old_property);
                    if (existing_prop) dynamic_cast<ResourceProperty<Cubemap>*>(property)->Set(existing_prop->Get());
                    break; }
                default:
                    // Unsupported currently
                    qDebug() << "Unsupported property type";
                    assert(false);
                    break;
            }

            if (property != nullptr) {
                 //qDebug()<<name.c_str()<<property<<"MAKE";
                Uniforms.AddProperty(name, property);
            }
        }
    }

    for (auto it : old_properties) {
        //qDebug() << it.first.c_str() << it.second << "DELET";
        delete it.second;
    }

    Uniforms.PropertiesChanged();
}

void Material::OnShaderProgramChanged() {
    OnShaderSet(Shader.Get());
}
