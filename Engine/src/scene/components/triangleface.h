#ifndef TRIANGLEFACE_H
#define TRIANGLEFACE_H

#include <scene/components/geometry.h>

class TriangleFace : public Geometry
{
public:
    TriangleFace(glm::vec3 a_, glm::vec3 b_, glm::vec3 c_,
                 glm::vec3 a_n_, glm::vec3 b_n_, glm::vec3 c_n_,
                 glm::vec2 a_uv_, glm::vec2 b_uv_, glm::vec2 c_uv_,
                 bool use_per_vertex_normals_=true);
    virtual ~TriangleFace() {};

    glm::dvec3 a, b, c;
    glm::vec3 a_n, b_n, c_n;
    glm::vec2 a_uv, b_uv, c_uv;
    bool use_per_vertex_normals;

    // This shouldn't be rasterized, only traced
    virtual Mesh* GetRenderMesh() { Q_ASSERT(false); return nullptr; }

    virtual bool UseCustomTrace() {
        return true;
    }

    virtual bool IntersectLocal(const Ray &r, Intersection &i);
};

#endif // TRIANGLEFACE_H
