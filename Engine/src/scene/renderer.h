/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef RENDERER_H
#define RENDERER_H

#include <scene/scene.h>

// Passed the Scene should batch and cache whatever to create it.
// *Decouple scene manager and renderer

// Traverses nodes and check if the Node is enabled, and0 if they have a MeshRenderer Component.
// If they do, check to see if there is a Mesh Component. Use the Material (and other Rendering settings from MeshRenderer)
// to create a VBO for that Mesh and VAO for that object.
// Ideally we'd want to batch, by having one VAO for each batch, but forget that for now.
class Renderer {
public:
    Renderer() :
        rendering_mode_(RenderingMode::Shaded),
        node_prefix_(""),
        material_override_(nullptr),
        default_texture_(nullptr),
        draw_lights_(true),
        draw_camera_(true),
        draw_colliders_(true),
        rendering_selection_(false)
    {}

    // Initialize renderer resources (Must only be called once)
    virtual void Initialize() = 0;
    // Clear the buffer before rendering each frame
    virtual void Clear() = 0;
    // Setup to render from the Scene (must be called again when Scene changes)
    virtual void Setup(Scene& scene) = 0;
    // Renders the node and its subtree additively
    virtual void RenderNode(SceneObject& node, const glm::mat4& view_matrix, const glm::mat4& proj_matrix, const glm::vec2& screen_size) = 0;

    // Prerender pass to render environment maps for reflections and shadowmaps
    // Wrapper function for saving render state; actual environment map rendering
    // is in a virtual function and must be implemented in subclasses.
    void RenderAllEnvMaps(SceneObject& root) {
        // Only render visible objects (meshes and particles)
        bool old_draw_camera = draw_camera_;
        draw_camera_ = false;
        bool old_draw_colliders = draw_colliders_;
        draw_colliders_ = false;
        bool old_draw_lights = draw_lights_;
        draw_lights_ = false;
        RenderingMode old_rendermode = rendering_mode_;
        SetRenderingMode(RenderingMode::Shaded);

        Material* old_override = material_override_;

        RenderEnvMaps(root);

        material_override_ = old_override;

        draw_camera_ = old_draw_camera;
        draw_colliders_ = old_draw_colliders;
        draw_lights_ = old_draw_lights;
        rendering_mode_ = old_rendermode;
    }


    // Toggles drawing of gizmos
    void DisplayLights(bool checked) { draw_lights_ = checked; }
    void DisplayCamera(bool checked) { draw_camera_ = checked; }
    void DisplayColliders(bool checked) { draw_colliders_ = checked; }

    // Draw either filled polygons, lines, or points
    void SetRenderingMode(RenderingMode mode) { rendering_mode_ = mode; }
    // Renders with the material specified instead of individual materials
    void SetMaterialOverride(Material* matoverride) { material_override_ = matoverride; }
    // Set if the renderer is drawing the selection wireframe
    void SetRenderingSelection(bool selection) { rendering_selection_ = selection; }
    // Renders with the default texture if one is not found
    void SetDefaultTexture(Texture* texture) { default_texture_ = texture; }
    // Renders only nodes with the given prefix
    void SetNodePrefix(std::string prefix="") { node_prefix_ = prefix; }

    void SetVertexEditing(bool edit) { vertex_editing_ = edit; }
protected:
    RenderingMode rendering_mode_;
    std::string node_prefix_;
    Material* material_override_;
    Texture* default_texture_;
    const SceneObject* ignored_node_;
    bool draw_lights_;
    bool draw_camera_;
    bool draw_colliders_;
    bool rendering_selection_;
    bool vertex_editing_;

    // Do not render this node or its children; used for environment mapping
    void SetIgnoredNode(const SceneObject* ignored) { ignored_node_ = ignored; }

    // Render environment maps for all scene nodes with EnvironmentMap
    // components
    virtual void RenderEnvMaps(SceneObject& root) = 0;
};

#endif // RENDERER_H
