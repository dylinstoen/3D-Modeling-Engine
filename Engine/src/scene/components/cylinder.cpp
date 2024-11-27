/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "cylinder.h"
#include <animator.h>

REGISTER_COMPONENT(Cylinder, Geometry)

Cylinder::Cylinder() :
    Geometry(),
    Subdivisions({"0", "1", "2", "3", "4", "5"}, 3)
{
    AddProperty("Subdivisions", &Subdivisions);

    Subdivisions.ValueSet.Connect(this, &Cylinder::OnSubdivisionsSet);
    RegenerateMesh();

    local_bbox.reset(new BoundingBox(glm::vec3(-0.5,-0.5,-0.5), glm::vec3(0.5,0.5,0.5)));
}


bool Cylinder::IntersectLocal( const Ray& r, Intersection& i )
{
    if( intersectCaps( r, i ) ) {
        Intersection ii;
        if( intersectBody( r, ii ) ) {
            if( ii.t < i.t ) {
                i = ii;
            }
        }
        return true;
    } else {
        return intersectBody( r, i );
    }
}

bool Cylinder::intersectBody( const Ray& r, Intersection& i ) const
{
    double x0 = r.position[0];
    double y0 = r.position[2];
    double x1 = r.direction[0];
    double y1 = r.direction[2];

    double a = x1*x1+y1*y1;
    double b = 2.0*(x0*x1 + y0*y1);
    double c = x0*x0 + y0*y0 - 0.5*0.5;

    if( 0.0 == a ) {
        // This implies that x1 = 0.0 and y1 = 0.0, which further
        // implies that the ray is aligned with the body of the cylinder,
        // so no intersection.
        return false;
    }

    double discriminant = b*b - 4.0*a*c;

    if( discriminant < 0.0 ) {
        return false;
    }

    discriminant = sqrt( discriminant );

    double t2 = (-b + discriminant) / (2.0 * a);

    if( t2 <= RAY_EPSILON ) {
        return false;
    }

    double t1 = (-b - discriminant) / (2.0 * a);

    if( t1 > RAY_EPSILON ) {
        // Two intersections.
        glm::dvec3 P = r.at( t1 );
        double z = P[1];
        if( z >= -0.5 && z <= 0.5 ) {
            // It's okay.
            i.t = t1;
            i.normal = glm::normalize(glm::vec3( P[0], 0.0, P[2] ));
            i.uv = glm::vec2(atan2(P[2],P[0])/glm::two_pi<double>() + 0.5, 0.5-P[1]);
            return true;
        }
    }

    glm::dvec3 P = r.at( t2 );
    double z = P[1];
    if( z >= -0.5 && z <= 0.5 ) {
        i.t = t2;
        i.normal = glm::normalize(glm::vec3( P[0], 0.0, P[2] ));
        i.uv = glm::vec2(atan2(P[2],P[0])/glm::two_pi<double>() + 0.5, 0.5-P[1]);
        return true;
    }

    return false;
}

bool Cylinder::intersectCaps( const Ray& r, Intersection& i ) const
{
    double pz = r.position[1];
    double dz = r.direction[1];

    if( 0.0 == dz ) {
        return false;
    }

    double t1;
    double t2;

    if( dz > 0.0 ) {
        t1 = (-0.5-pz)/dz;
        t2 = (0.5-pz)/dz;
    } else {
        t1 = (0.5-pz)/dz;
        t2 = (-0.5-pz)/dz;
    }

    if( t2 < RAY_EPSILON ) {
        return false;
    }

    if( t1 >= RAY_EPSILON ) {
        glm::dvec3 p( r.at( t1 ) );
        if( (p[0]*p[0] + p[2]*p[2]) <= 0.5*0.5 ) {
            i.t = t1;
            i.normal = glm::normalize(glm::vec3( 0.0, p[1], 0.0 ));
            i.uv = glm::vec2(0.5f+p[0], 0.5f-p[2]);
            return true;
        }
    }

    glm::dvec3 p( r.at( t2 ) );
    if( (p[0]*p[0] + p[2]*p[2]) <= 0.5*0.5 ) {
        i.t = t2;
        i.normal = glm::normalize(glm::vec3( 0.0, p[1], 0.0 ));
        i.uv = glm::vec2(0.5f+p[0], 0.5f-p[2]);
        return true;
    }

    return false;
}


void Cylinder::RegenerateMesh() {
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
std::unique_ptr<Mesh> Cylinder::CreateMesh(unsigned int subdivisions) {
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>("Cylinder");

    // Compute the amount of data we will produce
    unsigned int band_size = subdivisions + 1;
    unsigned int num_verts = band_size * 4 + 2;
    // Number of triangles in top and bottom circles plus sides
    unsigned int num_tris = (band_size - 1) * 4;

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

    // Generate the top and bottom caps
    float angle_step = (2 * M_PI) / subdivisions;
    float angle = 0.0f;
    // Top Center Vertex
    vertices[v_index++] = 0.0f;
    vertices[v_index++] = 0.5f;
    vertices[v_index++] = 0.0f;
    normals[n_index++] = 0.0f;
    normals[n_index++] = 1.0f;
    normals[n_index++] = 0.0f;
    UVs[uv_index++] = 0.5f;
    UVs[uv_index++] = 0.5f;
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
        // Top has indices: 0, 2, 4, 6 ...
        // Bot has indices: 1, 3, 5, 7 ...
        int band_index = (i*2) + 2;
        // Compute the vertex position by solving for points on the unit circle on plane XZ
        float cos_angle = glm::cos(angle);
        float sin_angle = glm::sin(angle);
        // Top
        vertices[v_index++] = 0.5f * cos_angle;
        vertices[v_index++] = 0.5f;
        vertices[v_index++] = 0.5f * sin_angle;
        normals[n_index++] = 0.0f;
        normals[n_index++] = 1.0f;
        normals[n_index++] = 0.0f;
        UVs[uv_index++] = 0.5f * cos_angle + 0.5f;
        UVs[uv_index++] = 0.5f * sin_angle + 0.5f;
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
            // Top
            triangles[tri_index++] = band_index - 2;
            triangles[tri_index++] = 0;
            triangles[tri_index++] = band_index;
            // Bottom
            triangles[tri_index++] = 1;
            triangles[tri_index++] = band_index - 1;
            triangles[tri_index++] = band_index + 1;
        }
        angle += angle_step;
    }

    // Generate the sides
    unsigned int sides_start = band_size * 2 + 2;
    angle = 0.0f;
    for (unsigned int i = 0; i < band_size; i++) {
        // Top has indices: 0, 2, 4, 6 ...
        // Bot has indices: 1, 3, 5, 7 ...
        int band_index = (i * 2) + sides_start;
        // Compute the vertex position by solving for points on the unit circle on plane XZ
        float cos_angle = glm::cos(angle);
        float sin_angle = glm::sin(angle);
        // Top
        vertices[v_index++] = 0.5f * cos_angle;
        vertices[v_index++] = 0.5f;
        vertices[v_index++] = 0.5f * sin_angle;
        // Normal is just the cos and sin normalized
        glm::vec3 normal = glm::normalize(glm::vec3(cos_angle, 0, sin_angle));
        normals[n_index++] = normal.x;
        normals[n_index++] = 0.0f;
        normals[n_index++] = normal.z;
        UVs[uv_index++] = angle / (2 * M_PI);
        UVs[uv_index++] = 1.0f;
        // Bottom
        vertices[v_index++] = 0.5f * cos_angle;
        vertices[v_index++] = -0.5f;
        vertices[v_index++] = 0.5f * sin_angle;
        normals[n_index++] = normal.x;
        normals[n_index++] = 0.0f;
        normals[n_index++] = normal.z;
        UVs[uv_index++] = angle / (2 * M_PI);
        UVs[uv_index++] = 0.0f;
        // Assemble the triangles
        if (i > 0) {
            SetQuad(triangles, tri_index, band_index - 1, band_index - 2, band_index, band_index + 1);
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
