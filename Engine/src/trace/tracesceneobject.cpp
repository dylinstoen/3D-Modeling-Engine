#include "tracesceneobject.h"

TraceGeometry::TraceGeometry(Geometry* geometry_, bool is_delete_geometry /*= false*/)
    : geometry(geometry_)
    , identity_transform(true)
    , transform(glm::mat4())
    , inverse_transform(glm::mat4())
    , normals_transform(glm::mat3())
    , m_is_delete_geometry(is_delete_geometry)

{
    world_bbox = geometry->HasBoundingBox() ? geometry->GetLocalBoundingBox() : nullptr;
}

TraceGeometry::TraceGeometry(Geometry* geometry_, glm::mat4 transform_, bool is_delete_geometry /*= false*/)
    : identity_transform(false)
    , transform(transform_)
    , inverse_transform(glm::inverse(transform_))
    , normals_transform(glm::transpose(glm::inverse(glm::mat3(transform_))))
    , geometry(geometry_)
    , m_is_delete_geometry(is_delete_geometry)
{
    world_bbox = geometry->HasBoundingBox() ? geometry->GetWorldBoundingBox(transform_) : nullptr;
}

TraceGeometry::~TraceGeometry()
{
    if (world_bbox) {
        delete world_bbox;
    }

    if (m_is_delete_geometry)
    {
        if (geometry)
            delete geometry;
    }
}

bool TraceGeometry::Intersect(const Ray &r, Intersection &i)
{
    //no transforms needed... nice
    if (identity_transform) {
        if (geometry->IntersectLocal(r, i)) {
            i.obj = this;
            return true;
        } else {
            return false;
        }
    }

    // Transform the ray into the object's local coordinate space
    glm::dvec3 pos = glm::dvec3(inverse_transform * glm::dvec4(r.position, 1));
    glm::dvec3 dir = glm::dvec3(inverse_transform * glm::dvec4((r.position + r.direction), 1)) - pos;
    double length = glm::length(dir);
    dir /= length;

    Ray localRay( pos, dir );

    if (geometry->IntersectLocal(localRay, i)) {
        // Transform the intersection point & normal returned back into global space.
        i.normal = glm::normalize(normals_transform * i.normal);
        i.t /= length;
        i.obj = this;
        return true;
    } else {
        return false;
    }
}

TraceFlare::TraceFlare(TraceLight *light) : trace_light(light)
{
    center = light->GetTransformPos();
    if (PointLight* point_light = dynamic_cast<PointLight*>(light->light)) {
        radius = std::max(point_light->TraceRadius.Get(), 0.001);

        world_bbox = new BoundingBox(center-glm::vec3{radius}, center+glm::vec3{radius});
    } else if (AreaLight* area_light = dynamic_cast<AreaLight*>(light->light)) {
        glm::vec3 p1 = (light->transform * glm::vec4(0.5f,0,0.5f,1)).xyz;
        glm::vec3 p2 = (light->transform * glm::vec4(-0.5f,0,0.5f,1)).xyz;
        glm::vec3 p3 = (light->transform * glm::vec4(0.5f,0,-0.5f,1)).xyz;
        glm::vec3 p4 = (light->transform * glm::vec4(-0.5f,0,-0.5f,1)).xyz;

        area = glm::length(glm::cross(p2-p4,p3-p4));

        world_bbox = new BoundingBox(glm::min(p1,glm::min(p2,glm::min(p3,p4))), glm::max(p1,glm::max(p2,glm::max(p3,p4))));
    } else {
        assert(false);
    }
}

TraceFlare::~TraceFlare()
{
    if (world_bbox) {
        delete world_bbox;
    }
}

bool TraceFlare::Intersect(const Ray &r, Intersection &i)
{
    glm::dvec3 ray2center = (glm::dvec3)center - r.position;
    if (PointLight* point_light = dynamic_cast<PointLight*>(trace_light->light)) {
        double dist_to_flare_plane = glm::dot(r.direction, ray2center);
        glm::dvec3 flare_plane_point_to_center = ray2center - r.direction * dist_to_flare_plane;

        if (dist_to_flare_plane > RAY_EPSILON && glm::length(flare_plane_point_to_center) < radius) {
            i.obj = this;
            i.t = dist_to_flare_plane;
            return true;
        }
    } else if (AreaLight* area_light = dynamic_cast<AreaLight*>(trace_light->light)) {
        glm::vec3 normal = (trace_light->transform * glm::vec4(0,1,0,0)).xyz;
        normal = normal/glm::length(normal);
        double dist_to_flare_plane = glm::dot((glm::dvec3)normal, ray2center);
        double step_to_flare_plane = glm::dot((glm::dvec3)normal, r.direction);
        double ray_dist_to_flare_plane = dist_to_flare_plane/step_to_flare_plane;
        glm::vec3 fpp = ray2center - r.direction * ray_dist_to_flare_plane;
        glm::vec3 u = (trace_light->transform * glm::vec4(1,0,0,0)).xyz;
        glm::vec3 v = (trace_light->transform * glm::vec4(0,0,1,0)).xyz;
        if (ray_dist_to_flare_plane > RAY_EPSILON && fabsf(glm::dot(fpp, u))<0.5f*glm::dot(u,u) && fabsf(glm::dot(fpp, v))<0.5f*glm::dot(v,v)) {
            i.obj = this;
            i.t = ray_dist_to_flare_plane;
            return true;
        }
    }

    return false;
}

glm::vec3 TraceFlare::GetIntensity(const Ray &r) {
    if (PointLight* point_light = dynamic_cast<PointLight*>(trace_light->light)) {
        glm::dvec3 ray2center = (glm::dvec3)center - r.position;
        double dist_to_flare_plane = glm::dot(r.direction, ray2center);
        glm::dvec3 flare_plane_point_to_center = ray2center - r.direction * dist_to_flare_plane;

        float denom = dynamic_cast<PointLight*>(trace_light->light)->AttenA.Get();
        float intensity_at_1 = denom > 0.0 ? 1.0 / denom : 1000.0;
        //Experimentally these seem to be the correct values, I don't know why PI is removed
        float dist_thru = 2.0f*sqrt((radius*radius) - glm::length2(flare_plane_point_to_center));
        float volume = (4.0/3.0)*radius*radius*radius; //(4.0/3.0)*M_PI*radius*radius*radius;
        return trace_light->light->GetIntensity() * intensity_at_1 * dist_thru/volume;
    } else if (AreaLight* area_light = dynamic_cast<AreaLight*>(trace_light->light)) {
        float denom = dynamic_cast<AreaLight*>(trace_light->light)->AttenA.Get();
        float intensity_at_1 = denom > 0.01 ? 1.0 / denom : 100.0;
        return glm::clamp(intensity_at_1 * trace_light->light->GetIntensity() / area, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
        //return intensity_at_1 * (float)M_PI * trace_light->light->GetIntensity() / area;
    }

    assert(false);
    return glm::vec3(0,0,0);
}