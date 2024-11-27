/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "resourceproperty.h"

#include <resource/assetmanager.h>
#include <resource/texture.h>

void ResourcePropertyBase::SetFromName(ResourcePropertyBase* obj, std::string name) {
    AssetManager& asset_manager = *(AssetManager::Instance());
    if (auto p = dynamic_cast<ResourceProperty<Texture>*>(obj)) {
        if (name == "") p->Set(nullptr);
        else p->Set(asset_manager.GetTexture(name));
    } else if (auto p = dynamic_cast<ResourceProperty<Cubemap>*>(obj)) {
        if (name == "") p->Set(nullptr);
        else p->Set(asset_manager.GetCubemap(name));
    } else if (auto p = dynamic_cast<ResourceProperty<Mesh>*>(obj)) {
        if (name == "") p->Set(nullptr);
        else p->Set(asset_manager.GetMesh(name));
    } else if (auto p = dynamic_cast<ResourceProperty<Material>*>(obj)) {
        if (name == "") p->Set(nullptr);
        else p->Set(asset_manager.GetMaterial(name));
    } else if (auto p = dynamic_cast<ResourceProperty<ShaderProgram>*>(obj)) {
        if (name == "") p->Set(nullptr);
        else p->Set(asset_manager.GetShaderProgram(name));
    } else {
        assert(false);
    }
}

//DO NOT call this from trace, it won't be thread safe
Texture *TextureProperty::Get() const {
    if (UseTexture.Get() && reinterpret_cast<ResourceProperty<Texture>*>(MappedColor)->IsSet()) {
        return reinterpret_cast<ResourceProperty<Texture>*>(MappedColor)->Get();
    } else {
        return solid_color_tex;
    }
}

void TextureProperty::SetColor(glm::vec3 color)
{
    SolidColor.Set(color);
    UseTexture.Set(false);
    SolidColorChanged({});
}

void TextureProperty::Set(Texture *tex)
{
    reinterpret_cast<ResourceProperty<Texture>*>(MappedColor)->Set(tex);
    UseTexture.Set(tex != nullptr);
}

void TextureProperty::SolidColorChanged(glm::vec4 unused)
{
    solid_color_tex = AssetManager::Instance()->GetOrCreateSolidTexture(SolidColor.GetRGB());
}

TextureProperty::TextureProperty(glm::vec3 colorDefault) : PropertyGroup(), UseTexture(), SolidColor(false), MappedColor(new ResourceProperty<Texture>(AssetType::Texture)) { //}, BilinearTexture() {

    AddProperty("Color", &SolidColor);
    AddProperty("Color Map", reinterpret_cast<ResourceProperty<Texture>*>(MappedColor));
    AddProperty("Use Texture", &UseTexture);
    //AddProperty("Bilinear Interpolation", &BilinearTexture);

    SolidColor.ValueSet.Connect(this, &TextureProperty::SolidColorChanged);
    UseTexture.ValueSet.Connect(this, &TextureProperty::OnUseTextureSet);

    UseTexture.Set(false); //is this necessary?
    SolidColor.Set(glm::vec4{colorDefault, 1.0});
    //BilinearTexture.Set(true);
}
