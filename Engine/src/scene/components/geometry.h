/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef MESHCOMPONENT_H
#define MESHCOMPONENT_H

#include <scene/components/component.h>
#include <resource/mesh.h>
#include <properties.h>
#include <resource/material.h>
#include <scene/boundingbox.h>
#include <trace/ray.h>

class Geometry : public Component
{
public:
    ResourceProperty<Material> RenderMaterial;

    Geometry() : RenderMaterial(AssetType::Material) {
        AddProperty("Material", &RenderMaterial);
        local_bbox = nullptr; //by default no bounding box is available
    }
    virtual ~Geometry() {}

    virtual Mesh* GetEditorMesh() { return GetRenderMesh(); }
    virtual Mesh* GetRenderMesh() = 0;

    bool HasBoundingBox() {
        return local_bbox != nullptr;
    }

    // Ownership to caller
    BoundingBox* GetLocalBoundingBox() {
        assert(HasBoundingBox());
        return new BoundingBox(local_bbox->min, local_bbox->max);
    }

    // Ownership to caller
    BoundingBox* GetWorldBoundingBox(glm::mat4 model_matrix) {
        assert(HasBoundingBox());
        return local_bbox->GetWorldBoundingBox(model_matrix);
    }

    // If false, it will just raytrace the render mesh
    virtual bool UseCustomTrace() {
        return false;
    }

    // Override this to define the custom trace
    virtual bool IntersectLocal(const Ray &, Intersection &) {
        Q_ASSERT(false);
        return false;
    }

protected:
    std::unique_ptr<BoundingBox> local_bbox;

private:
    std::map<int, std::unique_ptr<Mesh>> subdivision_cache;
    std::map<int, std::unique_ptr<Mesh>> limit_subdivision_cache;
    uint64_t subdivision_cache_mesh_uid;
    uint64_t subdivision_cache_mesh_version;
};

#endif // MESHCOMPONENT_H
