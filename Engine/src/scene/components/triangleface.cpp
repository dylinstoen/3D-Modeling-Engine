#include "triangleface.h"

TriangleFace::TriangleFace(glm::vec3 a_, glm::vec3 b_, glm::vec3 c_, glm::vec3 a_n_, glm::vec3 b_n_, glm::vec3 c_n_, glm::vec2 a_uv_, glm::vec2 b_uv_, glm::vec2 c_uv_, bool use_per_vertex_normals_) :
    a(a_), b(b_), c(c_), a_n(a_n_), b_n(b_n_), c_n(c_n_), a_uv(a_uv_), b_uv(b_uv_), c_uv(c_uv_), use_per_vertex_normals(use_per_vertex_normals_)
{
    local_bbox.reset(new BoundingBox(glm::min(a,glm::min(b,c)),glm::max(a,glm::max(b,c))));
}

bool TriangleFace::IntersectLocal(const Ray &r, Intersection &i)
{
   // TRACE: Add triangle intersection code here.
   // it currently ignores all triangles and just returns false.
   //
   // Note that you are only intersecting a single triangle, and the vertices
   // of the triangle are supplied to you by the trimesh class.
   //
   // use_per_vertex_normals tells you if the triangle has per-vertex normals.
   // If it does, you should compute and use the Phong-interpolated normal at the intersection point.
   // If it does not, you should use the normal of the triangle's supporting plane.
   //
   // If the ray r intersects the triangle abc:
   // 1. put the hit parameter in i.t
   // 2. put the normal in i.normal
   // 3. put the texture coordinates in i.uv
   // and return true;
   //
   return false;
}