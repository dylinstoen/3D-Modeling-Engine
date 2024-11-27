/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "cone.h"
#include <animator.h>

REGISTER_COMPONENT(Cone, Geometry)

Cone::Cone() :
    Geometry(),
    Subdivisions({"0", "1", "2", "3", "4", "5"}, 3)
{
    AddProperty("Subdivisions", &Subdivisions);

    Subdivisions.ValueSet.Connect(this, &Cone::OnSubdivisionsSet);
    RegenerateMesh();

    //local_bbox.reset(new BoundingBox(glm::vec3(-0.5,-0.5,-0.5), glm::vec3(0.5,0.5,0.5)));
}

void Cone::RegenerateMesh() {
    mesh_ = CreateMesh(std::pow(2, Subdivisions.Get() + 2));
}

inline void SetQuad(std::vector<unsigned int>& triangles, unsigned int& tri_index, unsigned int A, unsigned int B, unsigned int C, unsigned int D) {
    // Triangle ABD
    triangles[tri_index++] = A;
    triangles[tri_index++] = B;
    triangles[tri_index++] = D;
    // Triangle BCD
    triangles[tri_index++] = B;
    triangles[tri_index++] = C;
    triangles[tri_index++] = D;
}

// Transfers ownership of a new mesh to the caller
std::unique_ptr<Mesh> Cone::CreateMesh(unsigned int subdivisions) {
    // A cone is kind of like an Cylinder, except the top circle has radius 0.
    // Thus the sides don't need to be quads and the top has no triangles.
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>("Cone");

    // Compute the amount of data we will produce
    unsigned int band_size = subdivisions + 1;
    unsigned int num_verts = band_size * 3 + 1;
    // Number of triangles in bottom circle plus sides
    unsigned int num_tris = (band_size - 1) * 2;

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

    // Generate the bottom circle
    float angle_step = (2 * M_PI) / subdivisions;
    float angle = 0.0f;
    // Bottom Center Vertex
    vertices[v_index++] = 0.0f;
    vertices[v_index++] = -0.5f;
    vertices[v_index++] = 0.0f;
    normals[n_index++] = 0.0f;
    normals[n_index++] = -1.0f;
    normals[n_index++] = 0.0f;
    UVs[uv_index++] = 0.5f;
    UVs[uv_index++] = 0.5f;
    for (unsigned int i = 0; i < band_size; i++) {
        int band_index = i + 1;
        // Compute the vertex position by solving for points on the unit circle on plane XZ
        float cos_angle = glm::cos(angle);
        float sin_angle = glm::sin(angle);
        // Bottom
        vertices[v_index++] = 0.5f * cos_angle;
        vertices[v_index++] = -0.5f;
        vertices[v_index++] = 0.5f * sin_angle;
        normals[n_index++] = 0.0f;
        normals[n_index++] = -1.0f;
        normals[n_index++] = 0.0f;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Assemble the triangles
        if (i > 0) {
            // Bottom
            triangles[tri_index++] = 0;
            triangles[tri_index++] = band_index - 1;
            triangles[tri_index++] = band_index;
        }
        angle += angle_step;
    }

    // Calculate the normal for the first point
    glm::vec3 T1, T2, N;
    T1 = glm::vec3(0.0f, 0.5f, 0.0f) - glm::vec3(0.5f, 0.0f, 0.0f);
    T2 = glm::vec3(0, 0, 1);
    // T1 X T2 = N
    N = glm::cross(T1, T2);
    N = glm::normalize(N);

    // Generate the sides
    unsigned int sides_start = band_size + 1;
    angle = 0.0f;
    for (unsigned int i = 0; i < band_size; i++) {
        // Top has indices: 0, 2, 4, 6 ...
        // Bot has indices: 1, 3, 5, 7 ...
        int band_index = (i * 2) + sides_start;
        // Compute the vertex position by solving for points on the unit circle on plane XZ
        float cos_angle = glm::cos(angle);
        float sin_angle = glm::sin(angle);
        // Top
        vertices[v_index++] = 0.0f;
        vertices[v_index++] = 0.5f;
        vertices[v_index++] = 0.0f;
        // Generate the normal by rotating about the Y-Axis
        glm::vec3 normal;
        normals[n_index++] = 0;
        normals[n_index++] = 1;
        normals[n_index++] = 0;
        UVs[uv_index++] = angle / (2 * M_PI);
        UVs[uv_index++] = 1.0f;
        // Bottom
        vertices[v_index++] = 0.5f * cos_angle;
        vertices[v_index++] = -0.5f;
        vertices[v_index++] = 0.5f * sin_angle;
        normal.x = N.x * cos(-angle);
        normal.y = N.y;
        normal.z = N.x * -sin(-angle);
        normal = glm::normalize(normal);
        normals[n_index++] = normal.x;
        normals[n_index++] = normal.y;
        normals[n_index++] = normal.z;
        UVs[uv_index++] = angle / (2 * M_PI);
        UVs[uv_index++] = 0.0f;
        // Assemble the triangles
        if (i > 0) {
            triangles[tri_index++] = band_index - 1;
            triangles[tri_index++] = band_index;
            triangles[tri_index++] = band_index + 1;
        }
        angle += angle_step;
    }

    // Set the data on the mesh
    mesh->SetPositions(vertices);
    mesh->SetNormals(normals);
    mesh->SetUVs(UVs);
    mesh->SetTriangles(triangles);

    return std::move(mesh);
}

