/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "component.h"
#include <scene/scenemanager.h>
#include <stdexcept>
#include <QDebug>

std::map<std::string, MetaComponent*>* Component::typename_registry_ = nullptr;
std::map<std::type_index, MetaComponent*>* Component::typeinfo_registry_ = nullptr;

bool Component::IsDefined(std::string const classname) {
    return typename_registry_->find(classname) != typename_registry_->end();
}

std::string Component::GetTypeName() const {
    return GetMetaComponent()->TypeName;
}

std::type_index Component::GetBaseType() const {
    return GetMetaComponent()->BaseType;
}

MetaComponent const* Component::GetMetaComponent() const {
    MetaComponent* meta = (*typeinfo_registry_)[std::type_index(typeid(*this))];
    assert(meta != nullptr);
    return meta;
}

Component* Component::Create(std::string const ClassName) {
    return GetMetaComponent(ClassName)->Create();
}

MetaComponent const* Component::GetMetaComponent(std::string const ClassName) {
    MetaComponent* meta = (*typename_registry_)[ClassName];
    assert(meta != nullptr);
    return meta;
}
