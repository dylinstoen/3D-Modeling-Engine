/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "sphere.h"
#include <animator.h>

REGISTER_COMPONENT(Sphere, Geometry)

Sphere::Sphere()
   : Geometry()
   , Subdivisions({"0", "1", "2", "3", "4", "5"}, 4)
   , Roundness(0.5f, 0.0f, 0.5f, 0.01f)
{
    AddProperty("Quality", &Subdivisions);
    //AddProperty("Roundness", &Roundness);

    Subdivisions.ValueSet.Connect(this, &Sphere::OnSubdivisionsSet);
    //Roundness.ValueChanged.Connect(this, &Sphere::OnRoundnessSet);
    RegenerateMesh();

    local_bbox.reset(new BoundingBox(glm::vec3(-0.5,-0.5,-0.5), glm::vec3(0.5,0.5,0.5)));
}

bool Sphere::IntersectLocal(const Ray &r, Intersection &i)
{
    // TRACE: Sphere intersection code here.
    // Sphere is a unit sphere (radius = 0.5) centered at the origin.
    // Currently ignores all spheres and just returns false.
    // Hint: Use double-precision (not float) for the intersection test
    // If the ray r intersects this sphere:
    // 1. put the hit parameter in i.t
    // 2. put the normal in i.normal
    // 3. put the texture coordinates in i.uv
    // and return true;
    return false;
}

void Sphere::RegenerateMesh() {
    mesh_ = CreateMesh(Subdivisions.Get());
}

void SetVertex(std::vector<float>& vertices, unsigned int& v_index,
               std::vector<float>& normals, unsigned int& n_index,
               std::vector<float>& UVs, unsigned int& uv_index,
               float x, float y, float z, float roundness) {
    glm::vec3 outer(x, y, z);
    glm::vec3 inner(x, y, z);

    // Normal becomes undefined when outer == inner
    if (roundness == 0.0f) roundness = 0.0001f;

    float neg_boundary = roundness - 0.5f;
    float pos_boundary = 0.5f - roundness;

    if (x < neg_boundary) { inner.x = neg_boundary; }
    else if (x > pos_boundary) { inner.x = pos_boundary; }
    if (y < neg_boundary) { inner.y = neg_boundary; }
    else if (y > pos_boundary) { inner.y = pos_boundary; }
    if (z < neg_boundary) { inner.z = neg_boundary; }
    else if (z > pos_boundary) { inner.z = pos_boundary; }

    glm::vec3 normal = glm::normalize(outer - inner);
    glm::vec3 vertex = inner + normal * roundness;
    vertices[v_index++] = vertex.x;
    vertices[v_index++] = vertex.y;
    vertices[v_index++] = vertex.z;
    normals[n_index++] = normal.x;
    normals[n_index++] = normal.y;
    normals[n_index++] = normal.z;
    // TODO: Use cube mapped UV
    // See: https://forum.unity3d.com/threads/cube-mapped-sphere-aka-quad-sphere-asset.292860/
    float u = 0.5f + atan2(normal.z, normal.x) / (2 * M_PI);
    float v = 0.5f - asin(normal.y) / M_PI;
    UVs[uv_index++] = u;
    UVs[uv_index++] = (1 - v);
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

void SetTopFace(std::vector<unsigned int>& triangles, unsigned int& tri_index, unsigned int band_start, unsigned int grid_start, unsigned int width) {
    unsigned int band_end = band_start + (width - 1) * 4 - 1;

    // Inner grid
    unsigned int grid_width = width - 2;
    for (unsigned int j = 1 ; j < grid_width; j++) {
        for (unsigned int i = 1; i < grid_width; i++) {
            unsigned int A = (j) * grid_width + (i - 1) + grid_start;
            unsigned int B = (j - 1) * grid_width + (i - 1) + grid_start;
            unsigned int C = (j - 1) * grid_width + (i) + grid_start;
            unsigned int D = (j) * grid_width + (i) + grid_start;
            SetQuad(triangles, tri_index, A, B, C, D);
        }
    }

    // -X and +X Column
    unsigned int min_vertex = band_end;
    unsigned int max_vertex = band_start + width;
    for (unsigned int j = 1; j < grid_width; j++) {
        // Left Quad
        unsigned int A = min_vertex - 1;
        unsigned int B = min_vertex;
        unsigned int C = (j - 1) * grid_width + grid_start;
        unsigned int D = j * grid_width + grid_start;
        SetQuad(triangles, tri_index, A, B, C, D);
        // Right Quad
        A = (j) * grid_width + (grid_width - 1) + grid_start;
        B = (j - 1) * grid_width + (grid_width - 1) + grid_start;
        C = max_vertex;
        D = max_vertex + 1;
        SetQuad(triangles, tri_index, A, B, C, D);
        // Keep track of these
        min_vertex--;
        max_vertex++;
    }

    // +Z and -Z Rows
    min_vertex = band_start + 1;
    max_vertex = band_end - width + 1;
    for (unsigned int i = 1; i < grid_width; i++) {
        // Top Quad
        unsigned int A = (i - 1) + grid_start;
        unsigned int B = min_vertex;
        unsigned int C = min_vertex + 1;
        unsigned int D = i + grid_start;
        SetQuad(triangles, tri_index, A, B, C, D);
        // Bottom Quad
        A = max_vertex;
        B = (grid_width - 1) * grid_width + (i - 1) + grid_start;
        C = (grid_width - 1) * grid_width + i + grid_start;
        D = max_vertex - 1;
        SetQuad(triangles, tri_index, A, B, C, D);
        min_vertex++;
        max_vertex--;
    }

    // Corners
    SetQuad(triangles, tri_index, band_end, band_start, band_start + 1, grid_start); // -X, +Z
    SetQuad(triangles, tri_index, grid_start + (grid_width - 1), band_start + width - 2, band_start + width - 1, band_start + width); // +X, +Z
    SetQuad(triangles, tri_index, band_end - width + 2, band_end - width + 3, (grid_width - 1) * grid_width + grid_start, band_end - width + 1); // -X, -Z
    SetQuad(triangles, tri_index, band_end - width * 2 + 4, (grid_width - 1) * grid_width + (grid_width - 1) + grid_start, band_end - width * 2 + 2, band_end - width * 2 + 3); // +X, -Z
}

void SetBottomFace(std::vector<unsigned int>& triangles, unsigned int& tri_index, unsigned int band_start, unsigned int grid_start, unsigned int width) {
    unsigned int band_end = band_start + (width - 1) * 4 - 1;

    // Inner grid
    unsigned int grid_width = width - 2;
    for (unsigned int j = 1 ; j < grid_width; j++) {
        for (unsigned int i = 1; i < grid_width; i++) {
            unsigned int A = (j) * grid_width + (i - 1) + grid_start;
            unsigned int B = (j - 1) * grid_width + (i - 1) + grid_start;
            unsigned int C = (j - 1) * grid_width + (i) + grid_start;
            unsigned int D = (j) * grid_width + (i) + grid_start;
            SetQuad(triangles, tri_index, B, A, D, C);
        }
    }

    // -X and +X Column
    unsigned int min_vertex = band_end;
    unsigned int max_vertex = band_start + width;
    for (unsigned int j = 1; j < grid_width; j++) {
        // Left Quad
        unsigned int A = min_vertex - 1;
        unsigned int B = min_vertex;
        unsigned int C = (j - 1) * grid_width + grid_start;
        unsigned int D = j * grid_width + grid_start;
        SetQuad(triangles, tri_index, B, A, D, C);
        // Right Quad
        A = (j) * grid_width + (grid_width - 1) + grid_start;
        B = (j - 1) * grid_width + (grid_width - 1) + grid_start;
        C = max_vertex;
        D = max_vertex + 1;
        SetQuad(triangles, tri_index, B, A, D, C);
        // Keep track of these
        min_vertex--;
        max_vertex++;
    }

    // +Z and -Z Rows
    min_vertex = band_start + 1;
    max_vertex = band_end - width + 1;
    for (unsigned int i = 1; i < grid_width; i++) {
        // Top Quad
        unsigned int A = (i - 1) + grid_start;
        unsigned int B = min_vertex;
        unsigned int C = min_vertex + 1;
        unsigned int D = i + grid_start;
        SetQuad(triangles, tri_index, B, A, D, C);
        // Bottom Quad
        A = max_vertex;
        B = (grid_width - 1) * grid_width + (i - 1) + grid_start;
        C = (grid_width - 1) * grid_width + i + grid_start;
        D = max_vertex - 1;
        SetQuad(triangles, tri_index, B, A, D, C);
        min_vertex++;
        max_vertex--;
    }

    // Corners
    SetQuad(triangles, tri_index, band_start, band_end, grid_start, band_start + 1); // -X, +Z
    SetQuad(triangles, tri_index, band_start + width - 2, grid_start + (grid_width - 1), band_start + width, band_start + width - 1); // +X, +Z
    SetQuad(triangles, tri_index, band_end - width + 3, band_end - width + 2, band_end - width + 1, (grid_width - 1) * grid_width + grid_start); // -X, -Z
    SetQuad(triangles, tri_index, (grid_width - 1) * grid_width + (grid_width - 1) + grid_start, band_end - width * 2 + 4, band_end - width * 2 + 3, band_end - width * 2 + 2); // +X, -Z
}

// Transfers ownership of a new mesh to the caller
std::unique_ptr<Mesh> Sphere::CreateMesh(unsigned int subdivisions) {

    float radius = 0.5f;
    int sectorCount = 2 + std::pow(2, subdivisions);
    int stackCount = 2 + std::pow(2, subdivisions);

    float x, y, z, xz;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * M_PI / (float)sectorCount;
    float stackStep = M_PI / (float)stackCount;
    float sectorAngle, stackAngle;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - (float)i * stackStep;        // starting from pi/2 to -pi/2
        xz = radius * cosf(stackAngle);             // r * cos(u)
        y = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = -M_PI / 2 + (float)j * sectorStep;           // starting from -pi/2

            // vertex position (x, y, z)
            z = xz * cosf(sectorAngle);             // r * cos(u) * cos(v)
            x = xz * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);

            // vertex tex coord (s, t) range between [0, 1]
            s = 1.0 - (float)j / sectorCount;
            t = 1.0 - (float)i / stackCount;
            texCoords.push_back(s);
            texCoords.push_back(t);
        }
    }

    std::vector<unsigned int> indices;
    unsigned int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (stackCount-1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    std::unique_ptr<Mesh> surface = std::make_unique<Mesh>("Sphere");
    // Set the data on the mesh
    surface->SetPositions(vertices);
    surface->SetNormals(normals);
    surface->SetUVs(texCoords);
    surface->SetTriangles(indices);

    return std::move(surface);
}

// Transfers ownership of a new mesh to the caller
std::unique_ptr<Mesh> Sphere::CreateMesh_old(unsigned int subdivisions) {
    std::unique_ptr<Mesh> surface = std::make_unique<Mesh>("Sphere");
    float roundness = Roundness.Get();

    // Compute the amount of data we will produce
    const unsigned int num_corners = 8;
    const unsigned int num_faces = 6;
    const unsigned int num_edges = 12;
    unsigned int width = 1 + std::pow(2, subdivisions);
    // Number of vertices if we had 6 faces with (width^2) verts each, minus corner vertices duplicated, minus edge vertices duplicated
    unsigned int num_verts = num_faces * width * width - num_corners * 2 - num_edges * (width - 2);
    // Number of triangles in each face * number of faces
    unsigned int num_tris = num_faces * 2 * std::pow(4, subdivisions);

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

    // Create "bands" of vertices along the height axis
    for (unsigned int y = 0; y < width; y++) {
        float v = float(y) / (width - 1) - 0.5f;
        // +Z: from -X to +X
        for (unsigned int i = 0; i < width - 1; i++) {
            float u = float(i) / (width - 1) - 0.5f;
            // Calculate the position of the vertex
            SetVertex(vertices, v_index, normals, n_index, UVs, uv_index, u, v, 0.5f, roundness);
        }
        // +X: from +Z to -Z
        for (unsigned int i = 0; i < width - 1; i++) {
            float u = float(i) / (width - 1) - 0.5f;
            // Calculate the position of the vertex
            SetVertex(vertices, v_index, normals, n_index, UVs, uv_index, 0.5f, v, -u, roundness);
        }
        // -Z: from +X to -X
        for (unsigned int i = 0; i < width - 1; i++) {
            float u = float(i) / (width - 1) - 0.5f;
            // Calculate the position of the vertex
            SetVertex(vertices, v_index, normals, n_index, UVs, uv_index, -u, v, -0.5f, roundness);
        }
        // -X: from -Z to +Z
        for (unsigned int i = 0; i < width - 1; i++) {
            float u = float(i) / (width - 1) - 0.5f;
            // Calculate the position of the vertex
            SetVertex(vertices, v_index, normals, n_index, UVs, uv_index, -0.5f, v, u, roundness);
        }
    }

    // Cap the top: from -X to +X and +Z to -Z
    assert(v_index % 3 == 0);
    unsigned int top_start = v_index / 3; // Start of inner top face vertices
    for (unsigned int z = 1; z < width - 1; z++) {
        float v = float(z) / (width - 1) - 0.5f;
        for (unsigned int x = 1; x < width - 1; x++) {
            float u = float(x) / (width - 1) - 0.5f;
            // Calculate the position of the vertex
            SetVertex(vertices, v_index, normals, n_index, UVs, uv_index, u, 0.5f, -v, roundness);
        }
    }

    // Cap the bottom: from -X to +X and +Z to -Z
    unsigned int bot_start = v_index / 3; // Start of inner bot face vertices
    for (unsigned int z = 1; z < width - 1; z++) {
        float v = float(z) / (width - 1) - 0.5f;
        for (unsigned int x = 1; x < width - 1; x++) {
            float u = float(x) / (width - 1) - 0.5f;
            // Calculate the position of the vertex
            SetVertex(vertices, v_index, normals, n_index, UVs, uv_index, u, -0.5f, -v, roundness);
        }
    }

    // Assemble the triangles for the sides
    unsigned int num_verts_band = 4 * width - 4;
    for (unsigned int j = 1; j < width; j++) {
        for (unsigned int i = 1; i < num_verts_band + 1; i++) {
            unsigned int A = (j - 1) * num_verts_band + (i - 1);
            unsigned int B = (j) * num_verts_band + (i - 1);
            unsigned int C = (j) * num_verts_band + (i);
            unsigned int D = (j - 1) * num_verts_band + (i);
            // Last triangles need to re-use first vertex
            if (i == num_verts_band) {
                C = (j) * num_verts_band;
                D = (j - 1) * num_verts_band;
            }
            SetQuad(triangles, tri_index, B, A, D, C); // ABCD is CW not CCW
        }
    }

    // Assemble the triangles for the top and bottom faces
    if (subdivisions == 0) {
        // For a special case subdivs = 0, there are no top or bottom vertices generated
        SetQuad(triangles, tri_index, top_start - 1, top_start - 4, top_start - 3, top_start - 2);  // Top Face
        SetQuad(triangles, tri_index, 0, 3, 2, 1);  // Bottom Face
    } else {
        unsigned int last_band_start = top_start - num_verts_band;
        unsigned int first_band_start = 0;
        SetTopFace(triangles, tri_index, last_band_start, top_start, width);
        SetBottomFace(triangles, tri_index, first_band_start, bot_start, width);
    }

    // Set the data on the mesh
    surface->SetPositions(vertices);
    surface->SetNormals(normals);
    surface->SetUVs(UVs);
    surface->SetTriangles(triangles);

    return std::move(surface);
}
