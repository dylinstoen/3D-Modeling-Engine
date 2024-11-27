/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef GLMESH_H
#define GLMESH_H

#include <glinclude.h>
#include <resource/mesh.h>

class GLMesh : public Cacheable {
public:
    GLMesh(const Mesh& mesh);
    ~GLMesh();
    virtual void Render() const;
    void SetMeshData(const Mesh& mesh);
    unsigned int IndicesCount() const { return num_indices_; }
    unsigned int VerticesCount() const { return num_vertices_; }
protected:
    GLuint vertex_array_;
    GLuint elements_vbo_;
    GLuint position_vbo_;
    GLuint normal_vbo_;
    GLuint color_vbo_;
    GLuint UV_vbo_;
    GLuint binormal_vbo_;
    GLuint tangent_vbo_;
    unsigned int num_vertices_;
    unsigned int num_indices_;

    MeshType mesh_type_;
};

#endif // GLMESH_H
