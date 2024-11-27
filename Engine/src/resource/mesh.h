/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef MESH_H
#define MESH_H

#include <animator.h>
#include <resource/asset.h>
#include <properties.h>
#include <resource/cacheable.h>

// Mesh consists of triangles arranged in 3D space to create the impression of a solid object.
// A triangle is defined by its three corner points or vertices.
// See: https://docs.unity3d.com/Manual/AnatomyofaMesh.html
class Mesh : public Asset, public Cacheable {
public:

    // Relative Path to the asset on disk, should be set after loading from disk.
    // Some assets are stored in the scene file, in which case they will not have an external path.
    FileProperty ExternalPath;

    Mesh(const std::string& name, MeshType type = MeshType::Triangles);
    void SetPositions(const std::vector<float>& positions);
    void SetUVs(const std::vector<float>& UVs);
    void SetColors(const std::vector<float>& colors);
    void SetNormals(const std::vector<float>& normals);
    void SetBinormals(const std::vector<float>& binormals);
    void SetTangents(const std::vector<float>& tangents);
    void SetTriangles(const std::vector<unsigned int>& triangles);
    void CalculateBinormalsAndTangents();

    void Append(Mesh& other, glm::mat4 transform=glm::mat4());

    unsigned short GetVertexFormat() const;
    virtual AssetType GetType() const override { return AssetType::Mesh; }
    MeshType GetMeshType() const { return mesh_type_; }

    const std::vector<float>& GetPositions() const;
    const std::vector<float>& GetColors() const;
    const std::vector<float>& GetUVs() const;
    const std::vector<float>& GetNormals() const;
    const std::vector<float>& GetBinormals() const;
    const std::vector<float>& GetTangents() const;
    const std::vector<unsigned int>& GetTriangles() const;
private:
    // Triangle mesh, quad mesh, or other
    MeshType mesh_type_;

    // Dynamic or Static (or Streaming)
    // DrawType draw_type_;
    // TODO: Batching, group all Static objects into a single VBO.
    // Group all Dynamic objects with similar vertex format into same VBO.
    // For now, we use one VAO per mesh.
    // Vertex format can be defined as bit flags

    // TODO: Write this to be more robust, checks the arrays to make sure they all have the same number of vertices etc.
    // What if one of the vertex properties if not a float? How to handle the vertex format?
    std::vector<float> positions_;
    std::vector<float> colors_;
    std::vector<float> UVs_;
    std::vector<float> normals_;
    std::vector<float> binormals_;
    std::vector<float> tangents_;
    std::vector<unsigned int> triangles_;
};

#endif // MESH_H
