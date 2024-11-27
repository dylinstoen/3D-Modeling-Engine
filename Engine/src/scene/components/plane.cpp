/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "plane.h"

REGISTER_COMPONENT(Plane, Geometry)

Plane::Plane() :
    Geometry(),
    Subdivisions({"0", "1", "2", "3", "4", "5"})
{
    AddProperty("Subdivisions", &Subdivisions);

    Subdivisions.ValueSet.Connect(this, &Plane::OnSubdivisionsSet);
    OnSubdivisionsSet(Subdivisions.Get());
}

void Plane::OnSubdivisionsSet(int subdivisions) {
    mesh_ = CreateMesh(subdivisions);
}

// Transfers ownership of a new mesh to the caller
std::unique_ptr<Mesh> Plane::CreateMesh(unsigned int subdivisions) {
    std::unique_ptr<Mesh> surface = std::make_unique<Mesh>("Plane");

    // Compute the amount of data we will produce
    unsigned int num_tris = 2 * std::pow(4, subdivisions);
    unsigned int width = 1 + std::pow(2, subdivisions);
    unsigned int num_verts = width * width;

    // Draw two planes
    num_tris = 2 * num_tris;
    num_verts = 2 * num_verts;

    // Allocate arrays to contain our data
    std::vector<float> vertices(3 * num_verts);
    std::vector<float> normals(3 * num_verts);
    std::vector<float> UVs(2 * num_verts);
    std::vector<unsigned int> triangles(3 * num_tris);

    // Keep track of array indices
    unsigned int v_index = 0;
    unsigned int n_index = 0;
    unsigned int uv_index = 0;
    unsigned int tri_index = 0;

    // For each vertex <x, y>
    for (unsigned int y = 0; y < width; y++) {
        float v = float(y) / (width - 1);
        for (unsigned int x = 0; x < width; x++) {
            float u = float(x) / (width - 1);
            // Calculate the position of the vertex
            vertices[v_index++] = u - 0.5f; // X
            vertices[v_index++] = v - 0.5f; // Y
            vertices[v_index++] = 0.00001f; // Z
            // Calculate the normal of the vertex
            normals[n_index++] = 0; // X
            normals[n_index++] = 0; // Y
            normals[n_index++] = 1; // Z
            // Calculate the UV of the vertex
            UVs[uv_index++] = u;
            UVs[uv_index++] = v;
            // Construct the triangles
            if (x > 0 && y > 0) {
                unsigned int A = (y - 1) * width + (x - 1);
                unsigned int B = (y) * width + (x - 1);
                unsigned int C = (y) * width + (x);
                unsigned int D = (y - 1) * width + (x);
                // Triangle BAD
                triangles[tri_index++] = B;
                triangles[tri_index++] = A;
                triangles[tri_index++] = D;
                // Triangle CBD
                triangles[tri_index++] = C;
                triangles[tri_index++] = B;
                triangles[tri_index++] = D;
            }
        }
    }

    // Draw a second plane with reversed normals
    unsigned int backplane_start = v_index / 3;
    for (unsigned int y = 0; y < width; y++) {
        float v = float(y) / (width - 1);
        for (unsigned int x = 0; x < width; x++) {
            float u = float(x) / (width - 1);
            // Calculate the position of the vertex
            vertices[v_index++] = u - 0.5f; // X
            vertices[v_index++] = v - 0.5f; // Y
            vertices[v_index++] = -0.00001f; // Z
            // Calculate the normal of the vertex
            normals[n_index++] = 0; // X
            normals[n_index++] = 0; // Y
            normals[n_index++] = -1; // Z
            // Calculate the UV of the vertex
            UVs[uv_index++] = u;
            UVs[uv_index++] = v;
            // Construct the triangles
            if (x > 0 && y > 0) {
                unsigned int A = backplane_start + (y - 1) * width + (x - 1);
                unsigned int B = backplane_start + (y) * width + (x - 1);
                unsigned int C = backplane_start + (y) * width + (x);
                unsigned int D = backplane_start + (y - 1) * width + (x);
                // Triangle ABD
                triangles[tri_index++] = A;
                triangles[tri_index++] = B;
                triangles[tri_index++] = D;
                // Triangle BCD
                triangles[tri_index++] = B;
                triangles[tri_index++] = C;
                triangles[tri_index++] = D;
            }
        }
    }

    // Set the data on the mesh
    surface->SetPositions(vertices);
    surface->SetNormals(normals);
    surface->SetUVs(UVs);
    surface->SetTriangles(triangles);

    return std::move(surface);
}
