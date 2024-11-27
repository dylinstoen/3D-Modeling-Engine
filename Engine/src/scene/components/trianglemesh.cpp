#include "trianglemesh.h"

#include <meshprocessing.h>

REGISTER_COMPONENT(TriangleMesh, Geometry)

TriangleMesh::TriangleMesh() :
    Geometry(),
    MeshFilter(AssetType::Mesh),
    LoopSubdivision({"0", "1", "2", "3", "4", "5"}, 0)
{
    AddProperty("Mesh", &MeshFilter);
    AddProperty("LoopSubdivision", &LoopSubdivision);
}

Mesh* TriangleMesh::GetEditorMesh() {
    return MeshFilter.Get();
}

Mesh* TriangleMesh::GetRenderMesh() {
    if(MeshFilter.Get()==nullptr) {
        return nullptr;
    }

    uint64_t cur_uid = MeshFilter.Get()->GetUID();
    uint64_t cur_ver = MeshFilter.Get()->GetVersion();

    if (cur_uid != subdivision_cache_mesh_uid || cur_ver != subdivision_cache_mesh_version) {
        subdivision_cache.clear();
        limit_subdivision_cache.clear();
        subdivision_cache_mesh_uid = cur_uid;
        subdivision_cache_mesh_version = cur_ver;
    }

    int subdivisions = LoopSubdivision.Get();

    if (subdivisions==0) {
        return MeshFilter.Get();
    }

    int current_subdivision = 1;

    while (current_subdivision < subdivisions) {
        if (subdivision_cache.find(current_subdivision) == subdivision_cache.end()) {
            subdivision_cache[current_subdivision] = std::make_unique<Mesh>(MeshFilter.Get()->GetName()+" subdivision"+std::to_string(current_subdivision), MeshType::Triangles);
            MeshProcessing::SubdivideMesh(current_subdivision == 1 ?
                                              *(MeshFilter.Get()) :
                                              *(subdivision_cache[current_subdivision-1]),
                    *(subdivision_cache[current_subdivision]));
        }
        current_subdivision++;
    }
    if (limit_subdivision_cache.find(current_subdivision) == limit_subdivision_cache.end()) {
        limit_subdivision_cache[current_subdivision] =
            std::make_unique<Mesh>(MeshFilter.Get()->GetName()+" subdivision"+std::to_string(current_subdivision) + " limit", MeshType::Triangles);
        MeshProcessing::SubdivideMesh(current_subdivision == 1 ?
                *(MeshFilter.Get()) :
                *(subdivision_cache[current_subdivision-1]),
                *(limit_subdivision_cache[current_subdivision]),
                true);
    }

    return limit_subdivision_cache[subdivisions].get();
}
