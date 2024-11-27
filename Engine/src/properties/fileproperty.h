/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef FILEPROPERTY_H
#define FILEPROPERTY_H

#include <properties/property.h>

class FileProperty : public Property
{
public:
    Signal1<std::string> ValueSet;

    FileProperty(FileType type, const std::string& path = "");

    std::string Get() const;

    FileType GetFileType() const;
    void Set(std::string path);

    virtual bool IsSet() const override { return !path_.empty(); }

    virtual void SaveToYAML(YAML::Emitter& out) const override;
    virtual void LoadFromYAML(const YAML::Node& node) override;

private:
    std::string path_;
    FileType type_;
};

#endif // FILEPROPERTY_H
