#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <vectors.h>

#include <trace/ray.h>
//#include "trace/ray.h"

class BoundingBox
{
public:
    BoundingBox();
    BoundingBox(glm::vec3 min_, glm::vec3 max_);

    glm::vec3 min;
    glm::vec3 max;

    void operator=(const BoundingBox& target);
    void operator+=(const BoundingBox& target);

    // Does this bounding box intersect the target?
    bool intersects(const BoundingBox &target) const;

    // does the box contain this point?
    bool intersects(const glm::vec3& point) const;

    // OWNERSHIP TO CALLER
    BoundingBox* GetWorldBoundingBox(glm::mat4 local2world);

    bool IntersectRay(const Ray& r, double& tMin);

    float GetVolume() const { return glm::length(max - min); }

    glm::vec3 GetMid() const { return (max - min) * 0.5f + min; }

    float GetSurfaceArea() const {
        float w = max.x - min.x;
        float h = max.y - min.y;
        float d = max.z - min.z;
        return 2.f * (w * h + w * d + h * d);
    }

};

#endif // BOUNDINGBOX_H
