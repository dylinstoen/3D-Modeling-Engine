/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "yamlextensions.h"

namespace YAML {
    Emitter& operator << (YAML::Emitter& out, const glm::vec3& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    Emitter& operator << (YAML::Emitter& out, const glm::vec4& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    Emitter& operator << (YAML::Emitter& out, const glm::mat4& m) {
        out << YAML::Flow;
        out << YAML::BeginSeq;
        for (int j = 0; j < 4; j++) {
            for (int i = 0; i < 4; i++) {
                out << m[j][i];
            }
        }
        out << YAML::EndSeq;
        return out;
    }
}
