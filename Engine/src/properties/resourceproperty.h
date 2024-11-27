/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef RESOURCEPROPERTY_H
#define RESOURCEPROPERTY_H

#include <resource/asset.h>
#include "propertygroup.h"

#include <properties/property.h>

class Texture;

class ResourcePropertyBase : public Property {
public:
    // Needs to be a pointer so we can dynamic cast
    Signal1<Asset*> ValueSet;

    ResourcePropertyBase() : Property() {}

    virtual AssetType GetAssetType() const = 0;

    virtual Asset* GetAsset() const = 0;
    virtual void SetAsset(Asset* asset) = 0;

    static void SetFromName(ResourcePropertyBase* obj, std::string name);

    Asset* asset_;
    AssetType asset_type_;
};

template <typename ResourceType>
class ResourceProperty : public ResourcePropertyBase {
public:
    // TODO: Find some way to avoid passing in AssetType
    ResourceProperty(AssetType type, ResourceType* asset = nullptr) :
        ResourcePropertyBase()
    {
        asset_ = asset;
        asset_type_ = type;
    }

    ResourceType* Get() const { return dynamic_cast<ResourceType*>(asset_); }
    virtual void Set(ResourceType* asset) {
        asset_ = asset;
        if (asset_ != nullptr) asset_->Deleted.Connect(this, &ResourceProperty::AssetDeleted);
        if(allow_signals_) ValueSet.Emit(asset_);
    }

    virtual AssetType GetAssetType() const override { return asset_type_; }

    virtual Asset* GetAsset() const override { return asset_; }
    virtual void SetAsset(Asset* asset) override { Set(dynamic_cast<ResourceType*>(asset)); }

    virtual void SaveToYAML(YAML::Emitter& out) const override {
        if (asset_) {
            out << asset_->GetName();
        } else {
            out << "";
        }
    }
    virtual void LoadFromYAML(const YAML::Node& v) {
        std::string name = v.as<std::string>();

        ResourcePropertyBase::SetFromName(this, name);
    }

    virtual bool IsSet() const override { return asset_ != nullptr; }
private:
    void AssetDeleted() {
        asset_ = nullptr;
        if (allow_signals_) ValueSet.Emit(asset_);
    }


};

class TextureProperty : public PropertyGroup {
  public:
    BooleanProperty UseTexture;
    ColorProperty SolidColor;
    ResourcePropertyBase* MappedColor; //this is a pointer because of dependency loop bs

    TextureProperty(glm::vec3 colorDefault);

    ~TextureProperty() {
        delete MappedColor;
    }

    void OnUseTextureSet(bool use) {
        if (use) {
            SolidColor.SetHidden(use);
            MappedColor->SetHidden(!use);
        } else {
            MappedColor->SetHidden(!use);
            SolidColor.SetHidden(use);
        }
    }

    //DO NOT call this from a trace thread, it modifies the AssetManager
    Texture* Get() const;

    void SetColor(glm::vec3 color);
    void Set(Texture* tex);

    void SolidColorChanged(glm::vec4 unused);
  private:
    Texture* solid_color_tex;
};

#endif // RESOURCEPROPERTY_H
