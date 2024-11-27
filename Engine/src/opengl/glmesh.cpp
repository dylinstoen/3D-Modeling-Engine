/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <glextinclude.h>
#include "glmesh.h"
#include <opengl/glshaderprogram.h>

GLMesh::GLMesh(const Mesh& mesh) :
    Cacheable(&mesh)
{
    // Allocate a Vertex Array Object for this mesh
    glGenVertexArrays(1, &vertex_array_);

    // Allocate Vertex Buffer Objects for each vertex attribute and the triangle indices
    glGenBuffers(1, &elements_vbo_);
    glGenBuffers(1, &position_vbo_);
    glGenBuffers(1, &normal_vbo_);
    glGenBuffers(1, &color_vbo_);
    glGenBuffers(1, &UV_vbo_);
    glGenBuffers(1, &binormal_vbo_);
    glGenBuffers(1, &tangent_vbo_);

    SetMeshData(mesh);
}

void GLMesh::SetMeshData(const Mesh& mesh) {
    mesh_type_ = mesh.GetMeshType();
    glBindVertexArray(vertex_array_);
    auto attributes = GLShaderProgram::AttributeLocations();

    // Positions
    auto positions = mesh.GetPositions();
    GLint posAttrib = attributes["position"];
    if (positions.size() > 0) {
        num_vertices_ = positions.size()/3;
        glEnableVertexAttribArray(posAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, position_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW); // Static, Dynamic, or Stream
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    } else {
        glDisableVertexAttribArray(posAttrib);
    }

    // Normals
    auto normals = mesh.GetNormals();
    GLint nmlAttrib = attributes["normal"];
    if (normals.size() > 0) {
        glEnableVertexAttribArray(nmlAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normals.size(), normals.data(), GL_STATIC_DRAW); // Static, Dynamic, or Stream
        glVertexAttribPointer(nmlAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    } else {
        glDisableVertexAttribArray(nmlAttrib);
    }

    // Colors
    auto colors = mesh.GetColors();
    GLint colAttrib = attributes["color"];
    if (colors.size() > 0) {
        glEnableVertexAttribArray(colAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, color_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colors.size(), colors.data(), GL_STATIC_DRAW); // Static, Dynamic, or Stream
        glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    } else {
        glDisableVertexAttribArray(colAttrib);
    }

    // Texture Coords
    auto UVs = mesh.GetUVs();
    GLint texAttrib = attributes["texcoord"];
    if (UVs.size() > 0) {
        glEnableVertexAttribArray(texAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, UV_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * UVs.size(), UVs.data(), GL_STATIC_DRAW); // Static, Dynamic, or Stream
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);
    } else {
        glDisableVertexAttribArray(texAttrib);
    }

    // Binormals
    auto binormals = mesh.GetBinormals();
    GLint binmlAttrib = attributes["binormal"];
    if (binormals.size() > 0) {
        glEnableVertexAttribArray(binmlAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, binormal_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * binormals.size(), binormals.data(), GL_STATIC_DRAW); // Static, Dynamic, or Stream
        glVertexAttribPointer(binmlAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    } else {
        glDisableVertexAttribArray(binmlAttrib);
    }

    // Tangents
    auto tangents = mesh.GetTangents();
    GLint tngtAttrib = attributes["tangent"];
    if (tangents.size() > 0) {
        glEnableVertexAttribArray(tngtAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, tangent_vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tangents.size(), tangents.data(), GL_STATIC_DRAW); // Static, Dynamic, or Stream
        glVertexAttribPointer(tngtAttrib, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
    } else {
        glDisableVertexAttribArray(tngtAttrib);
    }

    // Triangles
    auto triangles = mesh.GetTriangles();
    // TODO: Watch out for meshes that have too many triangles or vertices.
    num_indices_ = triangles.size();
    if (num_indices_ > 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_vbo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * triangles.size(), triangles.data(), GL_STATIC_DRAW);
    }
    glBindVertexArray(0);
}

void GLMesh::Render() const {
    if (IndicesCount() > 0) {
        glBindVertexArray(vertex_array_);
        // Last parameter is a byte offset.
        if (mesh_type_ == MeshType::Triangles) glDrawElements(GL_TRIANGLES, IndicesCount(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    } else {
        // TODO: Non-indexed rendering
        glBindVertexArray(vertex_array_);
        if (mesh_type_ == MeshType::Lines) glDrawArrays(GL_LINES, 0, VerticesCount());
        glBindVertexArray(0);
    }
}

GLMesh::~GLMesh() {
    glDeleteVertexArrays(1, &vertex_array_);
    glDeleteBuffers(1, &elements_vbo_);
    glDeleteBuffers(1, &position_vbo_);
    glDeleteBuffers(1, &normal_vbo_);
    glDeleteBuffers(1, &color_vbo_);
    glDeleteBuffers(1, &UV_vbo_);
    glDeleteBuffers(1, &binormal_vbo_);
    glDeleteBuffers(1, &tangent_vbo_);
}
