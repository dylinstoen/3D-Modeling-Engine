#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H


#include <vectors.h>
#include <string>
#include <assert.h>
#include <signals/Signal.h>
#include <enum.h>
#include <map>
#include <memory>
#include <yaml-cpp/yaml.h>
#include <QDebug>

class Serializable
{
public:
    virtual ~Serializable() {}
    virtual void SaveToYAML(YAML::Emitter& out) const = 0;
    virtual void LoadFromYAML(const YAML::Node& node) = 0;
};

#endif // SERIALIZABLE_H
