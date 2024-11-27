#include "boundingbox.h"

BoundingBox::BoundingBox()
{
}

BoundingBox::BoundingBox(glm::vec3 min_, glm::vec3 max_) : min(min_), max(max_)
{
}

void BoundingBox::operator=(const BoundingBox& target) {
    min = target.min;
    max = target.max;
}

void BoundingBox::operator+=(const BoundingBox& target) {
    min = glm::min(min, target.min);
    max = glm::max(max, target.max);
}


// Does this bounding box intersect the target?
bool BoundingBox::intersects(const BoundingBox &target) const {
    return ((target.min[0] <= max[0]) && (target.max[0] >= min[0]) &&
            (target.min[1] <= max[1]) && (target.max[1] >= min[1]) &&
            (target.min[2] <= max[2]) && (target.max[2] >= min[2]));
}

// does the box contain this point?
bool BoundingBox::intersects(const glm::vec3& point) const {
    return ((point[0] >= min[0]) && (point[1] >= min[1]) && (point[2] >= min[2]) &&
            (point[0] <= max[0]) && (point[1] <= max[1]) && (point[2] <= max[2]));
}

BoundingBox* BoundingBox::GetWorldBoundingBox(glm::mat4 local2world)
{
    glm::vec4 v, newMax, newMin;

    v = local2world * glm::vec4(min[0], min[1], min[2], 1);
    newMax = v;
    newMin = v;
    v = local2world * glm::vec4(max[0], min[1], min[2], 1);
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = local2world * glm::vec4(min[0], max[1], min[2], 1);
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = local2world * glm::vec4(max[0], max[1], min[2], 1);
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = local2world * glm::vec4(min[0], min[1], max[2], 1);
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = local2world * glm::vec4(max[0], min[1], max[2], 1);
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = local2world * glm::vec4(min[0], max[1], max[2], 1);
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);
    v = local2world * glm::dvec4(max[0], max[1], max[2], 1);
    newMax = glm::max(newMax, v);
    newMin = glm::min(newMin, v);

    return new BoundingBox(newMin.xyz, newMax.xyz);
}

bool BoundingBox::IntersectRay(const Ray& r, double& tMin)
{
    tMin = -1.0e308; // 1.0e308 is close to infinity... close enough for us!
    double tMax = 1.0e308;

    const glm::dvec3& R0 = r.position;
    const glm::dvec3& Rd = r.direction;

    for (int currentaxis = 0; currentaxis < 3; currentaxis++)
    {
        double vd = Rd[currentaxis];

        // if the ray is parallel to the face's plane (=0.0)
        if( vd == 0.0 ) {
            if( R0[currentaxis] < min[currentaxis] || R0[currentaxis] > max[currentaxis] )
                return false;
            continue;
        }

        double v1 = min[currentaxis] - R0[currentaxis];
        double v2 = max[currentaxis] - R0[currentaxis];

        // two slab intersections
        double t1 = v1/vd;
        double t2 = v2/vd;

        if ( t1 > t2 ) {
            std::swap(t1, t2);
        }

        if (t1 > tMin)
            tMin = t1;
        if (t2 < tMax)
            tMax = t2;

        if (tMin > tMax) // box is missed
            return false;
        if (tMax < RAY_EPSILON) // box is behind ray
            return false;
    }

    return true;
}
