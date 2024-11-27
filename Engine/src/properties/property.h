/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef PROPERTY_H
#define PROPERTY_H

#include <vectors.h>
#include <string>
#include <assert.h>
#include <signals/Signal.h>
#include <enum.h>
#include <map>
#include <memory>
#include "serializable.h"

// For signal.h library
using namespace Gallant;

class Property : public virtual Serializable {
protected:
    Property() : locked_(false), hidden_(false), allow_signals_(true) {}
public:
    void BlockSignals() { allow_signals_ = false; }
    void UnblockSignals() { allow_signals_ = true; }

    virtual bool IsSet() const { return true; } // For properties such as FileProperty and ResourceProperty, which can be unset
    bool IsLocked() { return locked_; }
    void SetLocked(bool locked) { locked_ = locked; } // TODO: Setup events for this
    bool IsHidden() { return hidden_; }
    void SetHidden(bool hidden) {
        if (hidden_ == hidden) return;
        hidden_ = hidden;
        if (allow_signals_) HiddenChanged.Emit(hidden_);
    }

    Signal1<bool> HiddenChanged;
protected:
    bool locked_;
    bool hidden_;
    bool allow_signals_;
};

class ObjectWithProperties : public virtual Serializable
{
public:
    virtual void SaveToYAML(YAML::Emitter& out) const {
        out << YAML::BeginMap;
        for(auto it = properties_.begin(); it!=properties_.end(); it++) {
            out << YAML::Key << it->first << YAML::Value;
            it->second->SaveToYAML(out);
        }
        out << YAML::EndMap;
    }

    virtual void LoadFromYAML(const YAML::Node& node) {
       assert(node.IsMap());
       for(auto it = node.begin(); it!=node.end(); it++) {
           std::string key = it->first.as<std::string>();
           if (properties_.find(key) != properties_.end()) {
               properties_[key]->LoadFromYAML(it->second);
           } else {
               qDebug() << "Unknown key: " << key.c_str();
           }
       }
    }

    const std::vector<std::string>& GetProperties() const {
        return property_order_;
    }

    Property* GetProperty(std::string name) const {
        return properties_.find(name)==properties_.end() ? nullptr : properties_.at(name);
    }

    template<class PropertyType>
    PropertyType* Get(std::string name) const {
        return dynamic_cast<PropertyType*>(GetProperty(name));
    }

    void ClearProperties() {
        properties_.clear();
        property_order_.clear();
    }

    void AddProperty(std::string name, Property* member) {
        assert(properties_.find(name) == properties_.end());
        assert(member);
        properties_[name] = member;
        property_order_.push_back(name);
    }

protected:
    std::map<std::string, Property*> properties_;
    std::vector<std::string> property_order_;
};


#endif // PROPERTY_H
