#ifndef TRACELIGHT_H
#define TRACELIGHT_H

#include <scene/components/light.h>

// A simple object without hierarchical transformations and only a Light for fast tracing
class TraceLight
{
public:
    TraceLight(Light* light_, glm::mat4 transform_);
    ~TraceLight();

    Light* light;
    glm::mat4 transform; //local2world
    glm::mat4 inverse_transform;
    glm::mat3 normals_transform; //local2world

    //center of a point light
    glm::vec3 GetTransformPos() {
        return (transform * glm::vec4(0,0,0,1)).xyz;
    }

    //direction towards a directional light (opposite the light rays direction)
    glm::vec3 GetTransformDirection() {
        return (transform * glm::vec4(0,1,0,0)).xyz;
    }
};

#endif // TRACELIGHT_H
