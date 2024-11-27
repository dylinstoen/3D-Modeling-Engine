/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "mesh.h"
#include <algorithm>

Mesh::Mesh(const std::string& name, MeshType type) :
    Asset(name),
    ExternalPath(FileType::Mesh),
    mesh_type_(type)
{
    AddProperty("Path", &ExternalPath);
    ExternalPath.SetHidden(true);
}

void Mesh::SetPositions(const std::vector<float>& positions) {
    positions_ = std::vector<float>(positions);
    CalculateBinormalsAndTangents();
    MarkDirty();
}

void Mesh::SetUVs(const std::vector<float>& UVs) {
    UVs_ = std::vector<float>(UVs);
    CalculateBinormalsAndTangents();
    MarkDirty();
}

void Mesh::SetColors(const std::vector<float>& colors) {
    colors_ = std::vector<float>(colors);
    MarkDirty();
}

void Mesh::SetNormals(const std::vector<float>& normals) {
    normals_ = std::vector<float>(normals);
    CalculateBinormalsAndTangents();
    MarkDirty();
}

void Mesh::SetBinormals(const std::vector<float>& binormals) {
    binormals_ = std::vector<float>(binormals);
    MarkDirty();
}

void Mesh::SetTangents(const std::vector<float>& tangents) {
    tangents_ = std::vector<float>(tangents);
    MarkDirty();
}

void Mesh::SetTriangles(const std::vector<unsigned int>& triangles) {
    triangles_ = std::vector<unsigned int>(triangles);
    CalculateBinormalsAndTangents();
    MarkDirty();
}

void Mesh::Append(Mesh& other, glm::mat4 transform) {
    unsigned int sz1 = positions_.size()/3;
    unsigned int sz2 = other.positions_.size()/3;

    std::vector<float> remapped_pos(other.positions_.size());
    std::vector<float> remapped_norms(other.normals_.size());
    for (size_t i = 0; i < other.positions_.size(); i += 3) {
        glm::vec4 p(other.positions_[i],
                    other.positions_[i+1],
                    other.positions_[i+2],
                    1);
        p = transform*p;
        remapped_pos[i+0] = p[0];
        remapped_pos[i+1] = p[1];
        remapped_pos[i+2] = p[2];
    }
    for (size_t i = 0; i < other.normals_.size(); i += 3) {
        glm::vec4 n(other.normals_[i],
                    other.normals_[i+1],
                    other.normals_[i+2],
                    0);
        n = transform*n;
        remapped_norms[i+0] = n[0];
        remapped_norms[i+1] = n[1];
        remapped_norms[i+2] = n[2];
    }
    positions_.insert(positions_.end(), remapped_pos.begin(), remapped_pos.end());
    normals_.insert(normals_.end(), remapped_norms.begin(), remapped_norms.end());

    std::vector<float> remapped_binorms(other.binormals_.size());
    for (size_t i = 0; i < other.binormals_.size(); i += 3) {
        glm::vec4 n(other.binormals_[i],
                    other.binormals_[i+1],
                    other.binormals_[i+2],
                    0);
        n = transform*n;
        remapped_binorms[i+0] = n[0];
        remapped_binorms[i+1] = n[1];
        remapped_binorms[i+2] = n[2];
    }
    binormals_.insert(binormals_.end(), remapped_binorms.begin(), remapped_binorms.end());

    std::vector<float> remapped_tans(other.tangents_.size());
    for (size_t i = 0; i < other.tangents_.size(); i += 3) {
        glm::vec4 n(other.tangents_[i],
                    other.tangents_[i+1],
                    other.tangents_[i+2],
                    0);
        n = transform*n;
        remapped_tans[i+0] = n[0];
        remapped_tans[i+1] = n[1];
        remapped_tans[i+2] = n[2];
    }
    tangents_.insert(tangents_.end(), remapped_tans.begin(), remapped_tans.end());

    // UVs
    if (!UVs_.empty() || !other.UVs_.empty()) {
        UVs_.resize(2*(sz1 + sz2), 0.f);
        UVs_.insert(UVs_.begin()+2*sz1, other.UVs_.begin(), other.UVs_.end());
    }
    // Colors
    if (!colors_.empty() || !other.colors_.empty()) {
        colors_.resize(2*(sz1 + sz2), 0.f);
        colors_.insert(colors_.begin()+2*sz1, other.colors_.begin(), other.colors_.end());
    }

    std::vector<unsigned int> remapped_tris(other.triangles_.size());
    std::transform(other.triangles_.begin(), other.triangles_.end(), remapped_tris.begin(), [sz1](unsigned int x) { return x + sz1; });
    triangles_.insert(triangles_.end(), remapped_tris.begin(), remapped_tris.end());
    MarkDirty();
}

unsigned short Mesh::GetVertexFormat() const {
    unsigned short vertex_format = 0;
    if (positions_.size() > 0) vertex_format |= VF_POS;
    if (colors_.size() > 0) vertex_format |= VF_COL;
    if (UVs_.size() > 0) vertex_format |= VF_TEX;
    if (normals_.size() > 0) vertex_format |= VF_NRM;
    if (binormals_.size() > 0) vertex_format |= VF_BNM;
    if (tangents_.size() > 0) vertex_format |= VF_TAN;
    return vertex_format;
}

const std::vector<float>& Mesh::GetPositions() const {
    return positions_;
}

const std::vector<float>& Mesh::GetColors() const {
    return colors_;
}

const std::vector<float>& Mesh::GetUVs() const {
    return UVs_;
}

const std::vector<float>& Mesh::GetNormals() const {
    return normals_;
}

const std::vector<float>& Mesh::GetBinormals() const {
    return binormals_;
}

const std::vector<float>& Mesh::GetTangents() const {
    return tangents_;
}

const std::vector<unsigned int>& Mesh::GetTriangles() const {
    return triangles_;
}

void Mesh::CalculateBinormalsAndTangents() {
    if (UVs_.size()==0 || triangles_.size()==0 || (normals_.size()*2 != UVs_.size()*3) || (normals_.size() != positions_.size())) {
        return;
    }

    std::vector<float> binorms(normals_.size(), 0);
    std::vector<float> tans(normals_.size(), 0);

    //This turns out incorrect. I think one of the axises should be flipped. (TODO FIX)
    for (unsigned int i = 0; i < triangles_.size(); i += 3) {
        glm::vec3 P0(positions_[(triangles_[i]*3)], positions_[(triangles_[i]*3)+1], positions_[(triangles_[i]*3)+2]);
        glm::vec3 P1(positions_[(triangles_[i+1]*3)], positions_[(triangles_[i+1]*3)+1], positions_[(triangles_[i+1]*3)+2]);
        glm::vec3 P2(positions_[(triangles_[i+2]*3)], positions_[(triangles_[i+2]*3)+1], positions_[(triangles_[i+2]*3)+2]);

        glm::vec2 UV0(UVs_[(triangles_[i]*2)], UVs_[(triangles_[i]*2)+1]);
        glm::vec2 UV1(UVs_[(triangles_[i+1]*2)], UVs_[(triangles_[i+1]*2)+1]);
        glm::vec2 UV2(UVs_[(triangles_[i+2]*2)], UVs_[(triangles_[i+2]*2)+1]);

        //using Eric Lengyel's approach with a few modifications
        //from Mathematics for 3D Game Programmming and Computer Graphics
        // want to be able to trasform a vector in Object Space to Tangent Space
        // such that the x-axis cooresponds to the 's' direction and the
        // y-axis corresponds to the 't' direction, and the z-axis corresponds
        // to <0,0,1>, straight up out of the texture map

        //let P = v1 - v0
        glm::vec3 P = P1 - P0;
        //let Q = v2 - v0
        glm::vec3 Q = P2 - P0;
        float s1 = UV1.x - UV0.x;
        float t1 = UV1.y - UV0.y;
        float s2 = UV2.x - UV0.x;
        float t2 = UV2.y - UV0.y;

        //we need to solve the equation
        // P = s1*T + t1*B
        // Q = s2*T + t2*B
        // for T and B

        //this is a linear system with six unknowns and six equatinos, for TxTyTz BxByBz
        //[px,py,pz] = [s1,t1] * [Tx,Ty,Tz]
        // qx,qy,qz     s2,t2     Bx,By,Bz

        //multiplying both sides by the inverse of the s,t matrix gives
        //[Tx,Ty,Tz] = 1/(s1t2-s2t1) *  [t2,-t1] * [px,py,pz]
        // Bx,By,Bz                      -s2,s1	    qx,qy,qz

        //solve this for the unormalized T and B to get from tangent to object space
        glm::vec3 tan((t2*P.x - t1*Q.x), (t2*P.y - t1*Q.y), (t2*P.z - t1*Q.z));
        glm::vec3 binorm((s1*Q.x - s2*P.x), (s1*Q.y - s2*P.y), (s1*Q.z - s2*P.z));

        float factor = 1.0f/(s1*t2-s2*t1);

        tan = factor*tan;
        binorm = factor*binorm;

        for (unsigned int j = i; j <= i+2; j++) {
            binorms[(triangles_[j]*3)] += binorm.x;
            binorms[(triangles_[j]*3)+1] += binorm.y;
            binorms[(triangles_[j]*3)+2] += binorm.z;

            tans[(triangles_[j]*3)] += tan.x;
            tans[(triangles_[j]*3)+1] += tan.y;
            tans[(triangles_[j]*3)+2] += tan.z;
        }
    }

    // The binormal/tangent of each triangle around a vertex can be different, so just average them
    for (unsigned int i = 0; i < normals_.size(); i += 3) {
        glm::vec3 binorm(binorms[i], binorms[i+1], binorms[i+2]);
        glm::vec3 tan(tans[i], tans[i+1], tans[i+2]);

        binorm = glm::normalize(binorm);
        tan = glm::normalize(tan);

        binorms[i] = binorm.x;
        binorms[i+1] = binorm.y;
        binorms[i+2] = binorm.z;

        tans[i] = tan.x;
        tans[i+1] = tan.y;
        tans[i+2] = tan.z;
    }

    SetBinormals(binorms);
    SetTangents(tans);
}
