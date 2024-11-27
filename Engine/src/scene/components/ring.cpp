/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "ring.h"
#include <animator.h>

REGISTER_COMPONENT(Ring, Geometry)

Ring::Ring() :
    Geometry(),
    Subdivisions({"0", "1", "2", "3", "4", "5"}, 3),
    Width(0.05f, 0.0f, 1.0f, 0.01f)
{
    AddProperty("Subdivisions", &Subdivisions);
    AddProperty("Width", &Width);

    Subdivisions.ValueSet.Connect(this, &Ring::OnSubdivisionsSet);
    Width.ValueChanged.Connect(this, &Ring::OnWidthSet);
    RegenerateMesh();
}

void Ring::RegenerateMesh() {
    mesh_ = CreateMesh(std::pow(2, Subdivisions.Get() + 2), Width.Get());
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
std::unique_ptr<Mesh> Ring::CreateMesh(unsigned int subdivisions, double width) {
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>("Ring");

    // Compute the amount of data we will produce
    unsigned int band_size = subdivisions + 1;
    unsigned int num_verts = band_size * 8;
    // Number of triangles in sides plus top
    unsigned int num_tris = (band_size - 1) * 8;

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

    // Generate the top ring
    float inner_radius = 0.5f - width / 2.0f;
    float outer_radius = 0.5f + width / 2.0f;
    float angle_step = (2 * M_PI) / subdivisions;
    float angle = 0.0f;
    for (unsigned int i = 0; i < band_size; i++) {
        // Inner has indices: 0, 2, 4, 6 ...
        // Outer has indices: 1, 3, 5, 7 ...
        int band_index = i * 2;
        // Compute the vertex position by solving for points on the unit circle on plane XZ
        float cos_angle = glm::cos(angle);
        float sin_angle = glm::sin(angle);
        // Inner
        vertices[v_index++] = inner_radius * cos_angle;
        vertices[v_index++] = 0.5f;
        vertices[v_index++] = inner_radius * sin_angle;
        normals[n_index++] = 0.0f;
        normals[n_index++] = 1.0f;
        normals[n_index++] = 0.0f;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Bottom
        vertices[v_index++] = outer_radius * cos_angle;
        vertices[v_index++] = 0.5f;
        vertices[v_index++] = outer_radius * sin_angle;
        normals[n_index++] = 0.0f;
        normals[n_index++] = 1.0f;
        normals[n_index++] = 0.0f;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Assemble the triangles
        if (i > 0) {
            SetQuad(triangles, tri_index, band_index - 2, band_index - 1, band_index + 1, band_index);
        }
        angle += angle_step;
    }

    // Generate the bottom ring
    unsigned int bottom_start = band_size * 2;
    angle = 0.0f;
    for (unsigned int i = 0; i < band_size; i++) {
        // Inner has indices: 0, 2, 4, 6 ...
        // Outer has indices: 1, 3, 5, 7 ...
        int band_index = i * 2 + bottom_start;
        // Compute the vertex position by solving for points on the unit circle on plane XZ
        float cos_angle = glm::cos(angle);
        float sin_angle = glm::sin(angle);
        // Inner
        vertices[v_index++] = inner_radius * cos_angle;
        vertices[v_index++] = -0.5f;
        vertices[v_index++] = inner_radius * sin_angle;
        normals[n_index++] = 0.0f;
        normals[n_index++] = -1.0f;
        normals[n_index++] = 0.0f;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Bottom
        vertices[v_index++] = outer_radius * cos_angle;
        vertices[v_index++] = -0.5f;
        vertices[v_index++] = outer_radius * sin_angle;
        normals[n_index++] = 0.0f;
        normals[n_index++] = -1.0f;
        normals[n_index++] = 0.0f;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Assemble the triangles
        if (i > 0) {
            SetQuad(triangles, tri_index, band_index - 1, band_index - 2, band_index, band_index + 1);
        }
        angle += angle_step;
    }

    // Generate the outer
    unsigned int outer_start = band_size * 4;
    angle = 0.0f;
    for (unsigned int i = 0; i < band_size; i++) {
        // Inner has indices: 0, 2, 4, 6 ...
        // Outer has indices: 1, 3, 5, 7 ...
        int band_index = i * 2 + outer_start;
        // Compute the vertex position by solving for points on the unit circle on plane XZ
        float cos_angle = glm::cos(angle);
        float sin_angle = glm::sin(angle);
        // Top
        vertices[v_index++] = outer_radius * cos_angle;
        vertices[v_index++] = 0.5f;
        vertices[v_index++] = outer_radius * sin_angle;
        // Normal is just the cos and sin normalized
        glm::vec3 normal = glm::normalize(glm::vec3(cos_angle, 0, sin_angle));
        normals[n_index++] = normal.x;
        normals[n_index++] = 0.0f;
        normals[n_index++] = normal.z;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Bottom
        vertices[v_index++] = outer_radius * cos_angle;
        vertices[v_index++] = -0.5f;
        vertices[v_index++] = outer_radius * sin_angle;
        normals[n_index++] = normal.x;
        normals[n_index++] = 0.0f;
        normals[n_index++] = normal.z;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Assemble the triangles
        if (i > 0) {
            SetQuad(triangles, tri_index, band_index - 1, band_index - 2, band_index, band_index + 1);
        }
        angle += angle_step;
    }

    // Generate the inner
    unsigned int inner_start = band_size * 6;
    angle = 0.0f;
    for (unsigned int i = 0; i < band_size; i++) {
        // Inner has indices: 0, 2, 4, 6 ...
        // Outer has indices: 1, 3, 5, 7 ...
        int band_index = i * 2 + inner_start;
        // Compute the vertex position by solving for points on the unit circle on plane XZ
        float cos_angle = glm::cos(angle);
        float sin_angle = glm::sin(angle);
        // Top
        vertices[v_index++] = inner_radius * cos_angle;
        vertices[v_index++] = 0.5f;
        vertices[v_index++] = inner_radius * sin_angle;
        // Normal is negative of the cos and sin normalized
        glm::vec3 normal = -glm::normalize(glm::vec3(cos_angle, 0, sin_angle));
        normals[n_index++] = normal.x;
        normals[n_index++] = 0.0f;
        normals[n_index++] = normal.z;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Bottom
        vertices[v_index++] = inner_radius * cos_angle;
        vertices[v_index++] = -0.5f;
        vertices[v_index++] = inner_radius * sin_angle;
        normals[n_index++] = normal.x;
        normals[n_index++] = 0.0f;
        normals[n_index++] = normal.z;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
        // Assemble the triangles
        if (i > 0) {
            SetQuad(triangles, tri_index, band_index - 2, band_index - 1, band_index + 1, band_index);
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
