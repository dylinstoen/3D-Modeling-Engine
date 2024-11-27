/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "fileproperty.h"

FileProperty::FileProperty(FileType type, const std::string& path) :
    Property(),
    path_(path),
    type_(type)
{
}

std::string FileProperty::Get() const {
    return path_;
}

FileType FileProperty::GetFileType() const {
    return type_;
}

void FileProperty::Set(std::string path) {
    path_ = path;
    if (allow_signals_) ValueSet.Emit(path_);
}

void FileProperty::SaveToYAML(YAML::Emitter& out) const {
    out << path_;
}

void FileProperty::LoadFromYAML(const YAML::Node& node) {
    Set(node.as<std::string>());
}

