/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef ASSET_H
#define ASSET_H

#include <animator.h>

#include <properties/property.h>


// Asset represents things like Textures, Shaders, Materials, that are used by the Scene and SceneObjects.
// They may be created by external programs and stored on disk.
// See: https://docs.unity3d.com/Manual/AssetWorkflow.html
class Asset : public ObjectWithProperties {
public:
    Asset(const std::string& name) : uid_(++Asset::uid_counter_), name_(name), internal_(false), hidden_(false) {}
    ~Asset() {
        Deleted.Clear();
        NameChanged.Clear();
    }

    virtual AssetType GetType() const = 0;
    uint64_t GetUID() const { return uid_; }
    std::string GetName() const { return name_; }
    void SetName(std::string name) { name_ = name; NameChanged.Emit(name); }

    bool operator==(const Asset& other) const { return this->GetUID() == other.GetUID(); }
    bool operator!=(const Asset& other) const { return !(*this == other); }

    // If hidden, this asset does not show up in the hierarchy
    bool IsHidden() { return hidden_; }
    void SetHidden(bool hidden = true) { hidden_ = hidden; }

    // By default, all assets are external assets, meaning they are loaded from disk.
    // Internal assets are built into the program.
    void MakeInternal() { internal_ = true; }
    bool IsInternal() const { return internal_; }

    // Signals
    Signal1<std::string> NameChanged;
    Signal0<void> Deleted;

protected:
    static uint64_t uid_counter_; // Program might break if you make more than 9223372036854775807 objects
    uint64_t uid_; // Unique identifier among Assets
    std::string name_;
    bool internal_;
    // TODO: Serialize this property
    // TODO: Make UI responsive to this change (signal)
    bool hidden_;
};

#endif // ASSET_H
