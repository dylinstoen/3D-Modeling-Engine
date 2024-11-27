/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef COMPONENT_H
#define COMPONENT_H

#include <animator.h>
#include <vectors.h>
#include <properties.h>
#include <scene/scenemanager.h>

#include <QDebug>

class MetaComponent {
  public:
    MetaComponent(std::string const ClassName_, std::type_index const BaseClass_, std::string const BaseClassName_) :
        TypeName(ClassName_), BaseType(BaseClass_), BaseTypeName(BaseClassName_) {}
    virtual Component* Create() const = 0;
    std::string const TypeName;
    std::type_index const BaseType;
    std::string const BaseTypeName;
};

class Component : public ObjectWithProperties {
public:
    virtual ~Component() {}

    std::string GetTypeName() const;
    template<typename T> static std::string GetTypeName() {
        return GetMetaComponent<T>()->TypeName;
    }

    std::type_index GetBaseType() const;
    template<typename T> static std::type_index const GetBaseType() {
        if (typeinfo_registry_->find(std::type_index(typeid(T))) == typeinfo_registry_->end()) {
            return std::type_index(typeid(T)); // it is a virtual type, so it is the base type
        }
        return GetMetaComponent<T>()->BaseType;
    }

    static Component* Create(std::string const ClassName);
    template<typename T> static Component* Create() {
        return GetMetaComponent<T>()->Create();
    }

    static bool IsDefined(std::string const ClassName);

    template<typename T> static void Register(MetaComponent* meta) {
        if (typename_registry_ == nullptr) {
            typename_registry_ = new std::map<std::string, MetaComponent*>();
            typeinfo_registry_ = new std::map<std::type_index, MetaComponent*>();
        }
        (*typename_registry_)[meta->TypeName] = meta;
        (*typeinfo_registry_)[std::type_index(typeid(T))] = meta;
    }

    template<typename T>
    T* as() { return dynamic_cast<T*>(this); }

protected:
    static std::map<std::string, MetaComponent*>* typename_registry_;
    static std::map<std::type_index, MetaComponent*>* typeinfo_registry_;

    MetaComponent const* GetMetaComponent() const;
    static MetaComponent const* GetMetaComponent(std::string const ClassName);
    template<typename T> static MetaComponent const* GetMetaComponent() {
        MetaComponent* meta = (*typeinfo_registry_)[std::type_index(typeid(T))];
        assert(meta != nullptr);
        return meta;
    }
};

#define REGISTER_COMPONENT(ClassName_, BaseClass_) \
class Meta##ClassName_##Component : public MetaComponent { \
    public: \
        Meta##ClassName_##Component() : MetaComponent(#ClassName_, std::type_index(typeid(BaseClass_)), #BaseClass_) { \
            Component::Register<ClassName_>(this); \
        } \
        Component* Create() const { \
            return new ClassName_(); \
        } \
}; \
Meta##ClassName_##Component _Meta##ClassName_##Component_; \


#endif // COMPONENT_H
