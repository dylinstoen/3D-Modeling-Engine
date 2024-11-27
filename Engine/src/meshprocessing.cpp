/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "meshprocessing.h"
#include <algorithm>

void MeshProcessing::ComputeNormals(Mesh& mesh) {
    // MODELER: Recompute the normals for a mesh by taking a weighted
    // average of the normals of the adjacent faces
}

void MeshProcessing::FilterMesh(const Mesh& input_mesh, Mesh& filtered_mesh, double a) {
    const std::vector<float>& input_positions = input_mesh.GetPositions();
    const std::vector<float>& input_normals = input_mesh.GetNormals();
    const std::vector<float>& input_UVs = input_mesh.GetUVs();
    const std::vector<unsigned int>& input_faces = input_mesh.GetTriangles();

    // MODELER: Take a weighted sum of the vertex and its neighbors to produce a new mesh
    // with the same connectivity as the original mesh, but with updated vertex positions.
    // Vertices are neighbors if they share an edge. Filter weights will be 1 for the vertex,
    // and a / N for each neighboring vertex. The weights will then be normalized.
    // "a" controls smoothing or sharpening, and N is the number of neighboring vertices.

    filtered_mesh.SetPositions(input_positions);
    filtered_mesh.SetNormals(input_normals);
    filtered_mesh.SetUVs(input_UVs);
    filtered_mesh.SetTriangles(input_faces);
    ComputeNormals(filtered_mesh);
}

void MeshProcessing::SubdivideMesh(const Mesh& input_mesh, Mesh& filtered_mesh, bool limit) {
    const std::vector<float>& input_positions = input_mesh.GetPositions();
    const std::vector<float>& input_normals = input_mesh.GetNormals();
    const std::vector<float>& input_UVs = input_mesh.GetUVs();
    const std::vector<unsigned int>& input_faces = input_mesh.GetTriangles();

    // MODELER - EXTRA CREDIT (457): Perform one iteration of Loop subdivision on the mesh, assuming
    // the mesh is watertight (has no boundary vertices or edges).
    // For an extra bell, also detect and subdivide boundary vertices and edges

    filtered_mesh.SetPositions(input_positions);
    filtered_mesh.SetNormals(input_normals);
    filtered_mesh.SetUVs(input_UVs);
    filtered_mesh.SetTriangles(input_faces);

    ComputeNormals(filtered_mesh);
}

void MeshProcessing::FlipNormals(const Mesh& input_mesh, Mesh& filtered_mesh) {
    const std::vector<float>& input_normals = input_mesh.GetNormals();
    const std::vector<unsigned int>& input_faces = input_mesh.GetTriangles();
    std::vector<float> output_normals;
    std::vector<unsigned int> output_faces;

    for (unsigned int i = 0; i < input_normals.size(); i++) {
        output_normals.push_back(-input_normals[i]);
    }

    // Also need to change face winding order so that backfaces are now frontfaces
    for (unsigned int i = 0; i < input_faces.size(); i += 3) {
        output_faces.push_back(input_faces[i]);
        output_faces.push_back(input_faces[i+2]);
        output_faces.push_back(input_faces[i+1]);
    }

    filtered_mesh.SetPositions(input_mesh.GetPositions());
    filtered_mesh.SetNormals(output_normals);
    filtered_mesh.SetUVs(input_mesh.GetUVs());
    filtered_mesh.SetTriangles(output_faces);
}
