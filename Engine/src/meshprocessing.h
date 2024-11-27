/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef MESHPROCESSING_H
#define MESHPROCESSING_H
#include <resource/mesh.h>
#include <utility>

class MeshProcessing {
public:
    static void ComputeNormals(Mesh& mesh);
    static void FilterMesh(const Mesh& input_mesh, Mesh& filtered_mesh, double a);
    static void FlipNormals(const Mesh& input_mesh, Mesh& filtered_mesh);
    static void SubdivideMesh(const Mesh& input_mesh, Mesh& filtered_mesh, bool limit=false);

private:
    typedef std::pair<unsigned int, unsigned int> mesh_edge;
    static mesh_edge make_edge(unsigned int v1, unsigned int v2) {
        return v1 < v2 ? std::make_pair(v1, v2) : std::make_pair(v2, v1);
    }
};

#endif // MESHPROCESSING_H
