/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <animator.h>
#include <scene/components/component.h>
#include <scene/components/transform.h>
#include <functional>

#include <QDebug>
#include <serializable.h>

class Scene;

class SceneObject : public Serializable {
public:
    static const int NOT_INTERNAL = 0;
    static const int INTERNAL_HIDDEN = 1;
    static const int INTERNAL_SELECTABLE = 2;

    SceneObject(const std::string& name, int flag=NOT_INTERNAL);
    virtual ~SceneObject();

    template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr>
    T& AddComponent() {
        T* component_orig = new T();
        Component* component = dynamic_cast<Component*>(component_orig);
        AddComponent(component);
        return *component_orig;
    }

    Component* AddComponent(std::string classname);
    void AddComponent(Component* component);

    template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr>
    void RemoveComponent() {
        assert(Component::GetTypeName<T>() != "Transform");
        std::type_index type = Component::GetBaseType<T>();
        if (components_.count(type) > 0) {
            delete components_[type];
            components_.erase(type);
            ComponentRemoved.Emit(Component::GetTypeName<T>());
        }
    }

    template<typename T, typename std::enable_if<std::is_base_of<Component, T>::value>::type* = nullptr>
    T* GetComponent() {
        std::type_index type = Component::GetBaseType<T>();
        if (components_.find(type) == components_.end()) return nullptr;
        else return components_[type]->as<T>();
    }

    // Whether or not this object should be renderered
    bool IsEnabled() { return enabled_; }
    void SetEnabled(bool enabled)  { SetEnabled(enabled, true); }
    void SetEnabled(bool enabled, bool signal) {
        if (enabled_ == enabled) return;
        enabled_ = enabled;
        if (signal) EnabledChanged.Emit(enabled);
    }

    // If internal, won't be serialized
    bool IsInternal() { return flag_ != NOT_INTERNAL; }
    bool IsHidden() { return flag_ == INTERNAL_HIDDEN; }

    void SetFlag(int flag) {
        flag_ = flag;
        for (auto& kv : children_) {
            assert(kv.second != nullptr);
            kv.second->SetFlag(flag);
        }
    }

    std::string GetName() const { return name_; }
    void SetName(std::string name) {
        if (name_ == name || name.empty()) return;
        name_ = name;
        NameChanged.Emit(name);
    }

    uint64_t GetUID() const { return uid_; }
    Transform& GetTransform() {
        Transform* xform = GetComponent<Transform>();
        assert(xform != nullptr); // SceneObjects should always has a Transform component!
        return *xform;
    }
    // Used for traversing a scene graph
    // TODO: The best way to do this would actually be to return a custom iterator... but for now this will do at a performance cost.
    std::vector<SceneObject*> GetChildren() const {
        std::vector<SceneObject*> ret;
        for (auto& kv : children_) {
            assert(kv.second != nullptr); // A child should never be null
            ret.push_back(kv.second);
        }
        return ret;
    }

    // Utilities for finding descendants that satisfy some predicate
    std::vector<SceneObject*> FilterDescendants(std::function<bool(SceneObject*)> f) {
        std::vector<SceneObject*> ret;
        if (f(this)) ret.push_back(this);
        FilterDescendantsHelper(f, ret);
        return ret;
    }
    void FilterDescendantsHelper(std::function<bool(SceneObject*)> f, std::vector<SceneObject*>& ret) {
        for (auto& kv : children_) {
            assert(kv.second != nullptr); // A child should never be null
            if (f(kv.second)) ret.push_back(kv.second);
            kv.second->FilterDescendantsHelper(f, ret);
        }
    }
    SceneObject* FindDescendant(std::function<bool(SceneObject*)> f) {
        if (f(this)) return this;
        for (auto& kv : children_) {
            assert(kv.second != nullptr); // A child should never be null
            if (f(kv.second)) return kv.second;
            else {
                SceneObject* o = kv.second->FindDescendant(f);
                if (o != nullptr) return o;
            }
        }
        return nullptr;
    }

    SceneObject* FindDescendantByName(const std::string &name)
    {
        return FindDescendant(
            [&](SceneObject* p_obj){return (p_obj) && (p_obj->GetName()==name);}
        );
    }

    // Used for serialization and UI
    std::vector<Component*> GetComponents() const {
        std::vector<Component*> ret;
        for (auto& kv : components_) {
            assert(kv.second != nullptr); // A child should never be null
            ret.push_back(kv.second);
        }
        return ret;
    }

    // LocalToWorldMatrix - O(log(n)) operation
    glm::mat4 GetModelMatrix() {
        return GetParentModelMatrix() * GetTransform().GetMatrix();
    }

    glm::mat4 GetParentModelMatrix() {
        glm::mat4 model_matrix;
        SceneObject* parent = GetParent();
        while(parent != nullptr) {
            model_matrix = parent->GetTransform().GetMatrix() * model_matrix;
            parent = parent->GetParent();
        }
        return model_matrix;
    }

    // Sets the parent
    // Emits: ParentChanged if the parent is successfully changed.
    void SetParent(SceneObject& parent) {
        SceneObject* old_parent = parent_;
        OnSetParent(parent);
        if (old_parent != parent_) ParentChanged.Emit(*parent_);
    }

    // Should not be called except by SetParent!
    // TODO: How do we get rid of the public method? Hmm...
    // THOUGHT: We could make this a private function that we register in the Scene when we are created
    void RegisterChild(SceneObject& child) {
        children_[child.GetUID()] = &child;
    }

    // Should not be called except by SetParent!
    // TODO: How do we get rid of the public method? Hmm...
    void RemoveChild(uint64_t uid) {
        auto it = children_.find(uid);
        if (it != children_.end()) children_.erase(it);
    }

    // TODO: Find Child
    SceneObject* GetParent() {
        return parent_;
    }

    // For convenience
    bool operator==(const SceneObject &other) const {
        return this->GetUID() == other.GetUID();
    }

    bool operator!=(const SceneObject &other) const {
        return !(*this == other);
    }

//    // Called when a new Scene is loaded so SceneObjects for that scene are generated from 0.
//    // TODO: Find a better method that would support more than one scene loaded at the same time.
//    // i.e. Root is not limited to being UID 0
//    static void ResetUIDCounter() { uid_counter_ = 0; }

    // Signals
    Signal1<Component&> ComponentAdded;
    Signal1<std::string> ComponentRemoved;
    Signal1<SceneObject&> LightSourceAdded;
    Signal1<SceneObject&> ParticleSystemAdded;
    Signal1<SceneObject&> ConnectedParticleSystemAdded;
    Signal1<std::string> NameChanged;
    Signal1<bool> EnabledChanged;
    Signal1<SceneObject&> ParentChanged;

    void SetParticleGeom(std::string name);

    virtual void SaveToYAML(YAML::Emitter& out) const;
    virtual void LoadFromYAML(const YAML::Node& node);

protected:
    // Shouldn't allow the parent to be set to null, so use a reference.
    // Only the root node has its parent as null.
    void OnSetParent(SceneObject& parent) {
        if (uid_ == 0) return; // If we are the root, we cannot have a parent!
        assert(!parent.IsInternal() || IsInternal());
        if (parent_ != nullptr) {
            if (*parent_ == parent) return;     // Make sure that the parent is not itself!
            else parent_->RemoveChild(uid_);    // Original parent needs to forget its child.
        }
        // Set the parent relationship and notify the parent of its new child.
        parent_ = &parent;
        parent_->RegisterChild(*this);
    }

    static uint64_t uid_counter_; // Program might break if you make 9223372036854775807 objects
    uint64_t uid_; // Unique identifier for this scene object
    // TODO: Maybe name, enabled should be BooleanProperty and TextProperty rather than raw as they are now.
    std::string name_;

    std::map<std::type_index, Component*> components_;
    SceneObject* parent_; // Nullptr if this node is the root
    Scene* scene_; // Can't be a ref cause refs must be initialized
    std::map<uint64_t, SceneObject*> children_;

    // TODO: Serialize this property
    // TODO: Make UI responsive to this change (signal)
    bool enabled_; // Whether or not the components in this object are active
    int flag_;
};

#endif // SCENEOBJECT_H
