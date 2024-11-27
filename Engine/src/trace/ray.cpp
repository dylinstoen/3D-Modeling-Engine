/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "ray.h"
#include "tracesceneobject.h"

#include <scene/components/triangleface.h>

Material* Intersection::GetMaterial()
{
   assert(obj != nullptr);
   TraceGeometry* geo = dynamic_cast<TraceGeometry*>(obj);
   assert(geo != nullptr);
   return geo->geometry->RenderMaterial.Get();
}

glm::vec3 Intersection::GetTrueNormal()
{
    assert(obj != nullptr);
    TraceGeometry* geo = dynamic_cast<TraceGeometry*>(obj);
    assert(geo != nullptr);
    Geometry* geo2 = geo->geometry;
    if (TriangleFace* faec = dynamic_cast<TriangleFace*>(geo2)) {
        glm::dvec3 ab = faec->b - faec->a;
        glm::dvec3 ac = faec->c - faec->a;
        return glm::normalize(glm::cross(ab, ac));
    }
    return normal;
}
