/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef YAMLEXTENSIONS_H
#define YAMLEXTENSIONS_H

#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>

// Add support in the YAML library for our own types
namespace YAML {
    Emitter& operator << (YAML::Emitter& out, const glm::vec3& v);

    Emitter& operator << (YAML::Emitter& out, const glm::vec4& v);

    Emitter& operator << (YAML::Emitter& out, const glm::mat4& m);

    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs) {
            if(!node.IsSequence() || node.size() != 3) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4> {
        static Node encode(const glm::vec4& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs) {
            if(!node.IsSequence() || node.size() != 4) {
                return false;
            }

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::mat4> {
        static Node encode(const glm::mat4& rhs) {
            Node node;
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    node.push_back(rhs[j][i]);
                }
            }
            return node;
        }

        static bool decode(const Node& node, glm::mat4& rhs) {
            if(!node.IsSequence() || node.size() != 16) {
                return false;
            }

            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    rhs[j][i] = node[j * 4 + i].as<float>();
                }
            }
            return true;
        }
    };
}
#endif // YAMLEXTENSIONS_H
