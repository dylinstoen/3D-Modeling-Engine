/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "propertygroup.h"
#include <algorithm>

/*
PropertyGroup::PropertyGroup(std::vector<std::pair<std::string,Property*>> properties, bool owns_pointers) :
    Property()
{
    SetProperties(properties, owns_pointers);
}

PropertyGroup::~PropertyGroup()
{
    SetProperties(std::vector<std::pair<std::string,Property*>>{}); //delete properties if we should
}

const std::vector<std::string>& PropertyGroup::GetProperties() const {
    return properties_order_;
}

void PropertyGroup::SetProperties(const std::vector<std::pair<std::string,Property*>>& properties, bool owns_pointers) {
    if (owns_pointers_) {
        for (int i = 0; i<properties_order_.size(); i++) {
            std::string label = properties_order_[i];
            bool in_next = false;
            for (int j = 0; j<properties.size(); j++) {
                if (serializable_members_[label]==properties[j].second) {
                    in_next = true;
                    break;
                }
            }
            if (!in_next) {
                delete props_vector_[i];
            }
        }
    }

    owns_pointers_ = owns_pointers;

    properties_order_.clear();
    std::map<std::string, Serializable*> membs;

    for (auto& property : properties) {
        assert(property.second);
        membs[property.first] = property.second;
        properties_order_.push_back(property.first);
    }

    SetMembers(membs);

    if (allow_signals_) ValueSet.Emit(properties);
}

Property* PropertyGroup::GetProperty(const std::string& name) {
    return serializable_members_.find(name)!=serializable_members_.end() ? dynamic_cast<Property*>(serializable_members_[name]) : nullptr;
}*/

