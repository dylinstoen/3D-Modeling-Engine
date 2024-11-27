#include "tracelight.h"

TraceLight::TraceLight(Light* light_, glm::mat4 transform_) :
    light(light_), transform(transform_), inverse_transform(glm::inverse(transform_)), normals_transform(glm::transpose(glm::inverse(glm::mat3(transform_))))
{
}

TraceLight::~TraceLight()
{
}

