/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include <vectors.h>
#include <scene/renderer.h>
#include <opengl/glerror.h>
#include <opengl/glshaderprogram.h>
#include <opengl/glresourcemanager.h>

// See: https://www.khronos.org/opengl/wiki/Vertex_Specification_Best_Practices
class GLRenderer : public Renderer {
public:
    GLRenderer();

    static void GlobalInitialize();
    virtual void Initialize() override;
    virtual void Clear() override;
    virtual void Setup(Scene& scene) override;
    virtual void RenderNode(SceneObject& node, const glm::mat4& view_matrix, const glm::mat4& proj_matrix, const glm::vec2& screen_size) override;

    void ContextChanged() { resource_manager_ = GLResourceManager(); }
protected:
    AssetManager* asset_manager_;
    GLResourceManager resource_manager_;
    std::stack<glm::mat4> matrix_stack_; // There's not really a reason to use this stack other than to be explicit, since we could use the callstack instead
    glm::mat4 model_matrix_;
    glm::mat4 view_matrix_;
    glm::mat4 proj_matrix_;
    glm::vec2 screen_size_;
    GLuint last_highest_tex = 0;
    std::vector<std::pair<SceneObject*, glm::mat4>> point_lights_;
    std::vector<std::pair<SceneObject*, glm::mat4>> dir_lights_;
    std::vector<std::pair<SceneObject*, glm::mat4>> spot_lights_;
    std::vector<std::pair<SceneObject*, glm::mat4>> area_lights_;
    std::vector<std::pair<SceneObject*, glm::mat4>> env_maps_;

    virtual void RenderEnvMaps(SceneObject& root);

    void Render(SceneObject& node);
    void Render(SceneObject& node, SphereCollider& collider);
    void Render(SceneObject& node, PlaneCollider& collider);
    void Render(SceneObject& node, CylinderCollider& collider);
    void Render(SceneObject& node, PointLight& light);
    void Render(SceneObject& node, DirectionalLight& light);
    void Render(SceneObject& node, SpotLight& light);
    void Render(SceneObject& node, AreaLight& light);
    void Render(SceneObject& node, Camera& rendercam);
    void Render(SceneObject& node, Geometry& renderer);
    void Render(SceneObject& node, ParticleSystem& particles);
    void Render(SceneObject& node, ConnectedParticleSystem& particles);
    void RenderDeformedMesh(SceneObject& node, std::vector<glm::vec3> points);
    void SetUniforms(GLShaderProgram& shader, Material& material, SceneObject& node);
};

#endif // OPENGLRENDERER_H
