/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include <properties.h>
#include <scene/components/geometry.h>


class TriangleMesh : public Geometry
{
public:
    ResourceProperty<Mesh> MeshFilter;
    ChoiceProperty LoopSubdivision;

    TriangleMesh();

    virtual Mesh* GetEditorMesh();
    virtual Mesh* GetRenderMesh();

private:
    std::map<int, std::unique_ptr<Mesh>> subdivision_cache;
    std::map<int, std::unique_ptr<Mesh>> limit_subdivision_cache;
    uint64_t subdivision_cache_mesh_uid = 0;
    uint64_t subdivision_cache_mesh_version = 0;
};


#endif // TRIANGLEMESH_H
