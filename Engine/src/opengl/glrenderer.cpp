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
#include "glrenderer.h"
#include <opengl/glmesh.h>
#include <opengl/gltexture2d.h>
#include <opengl/glrenderablecubemap.h>
#include <opengl/glshaderprogram.h>
#include <glm/gtx/matrix_decompose.hpp>

#include <scene/components/camera.h>
#include <trace/raytracer.h>

GLRenderer::GLRenderer() :
    asset_manager_(nullptr)
{

}

void GLRenderer::Initialize() {
    // GlobalInitialize();
}

void GLRenderer::GlobalInitialize() {
#ifndef __APPLE__
    // Force GLEW to use modern OpenGL method for checking function availability
    glewExperimental = GL_TRUE;
    // Initialize GLEW (must be called after OpenGL context is created and made current!)
    glewInit();
#endif

    // Display status
    std::ostringstream glStatus;
    glStatus << "OpenGL initialized: version "<< glGetString(GL_VERSION) << " GLSL "<< glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    Debug::Log.WriteLine(glStatus.str());
    glGetError(); // Remove superfluous gl error from glewInit
}

void GLRenderer::Clear() {
    // Draw the scene:
    glEnable(GL_MULTISAMPLE); // MSAA does not seem to be working
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glDepthFunc (GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRenderer::Setup(Scene& scene) {
    asset_manager_ = &scene.GetAssetManager();

    // Draw either filled triangles, triangles edges, or just triangle vertices
    switch(rendering_mode_) {
        case RenderingMode::Shaded:
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            break;
        case RenderingMode::Wireframe:
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            break;
        case RenderingMode::Points:
            glPointSize(3.0f);
            glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
            break;
    }

    scene.RenderPrepass();

    env_maps_ = scene.GetEnvMaps();

    point_lights_.clear();
    dir_lights_.clear();
    spot_lights_.clear();
    area_lights_.clear();
    // Split lights by type
    std::vector<std::pair<SceneObject*, glm::mat4>> lights = scene.GetLights();
    for (auto& kv : lights) {
        auto scene_object = kv.first;
        Light* light = scene_object->GetComponent<Light>();
        if (PointLight* p_light = light->as<PointLight>()) point_lights_.push_back(kv);
        else if (DirectionalLight* d_light = light->as<DirectionalLight>()) dir_lights_.push_back(kv);
        else if (SpotLight* s_light = light->as<SpotLight>()) spot_lights_.push_back(kv);
        else if (AreaLight* a_light = light->as<AreaLight>()) area_lights_.push_back(kv);
    }
}

void GLRenderer::RenderNode(SceneObject& node, const glm::mat4& view_matrix, const glm::mat4& proj_matrix, const glm::vec2& screen_size) {
    // Reset the matrices
    model_matrix_ = glm::mat4();
    view_matrix_ = view_matrix;
    proj_matrix_ = proj_matrix;
    screen_size_ = screen_size;

    // Calculate the model_matrix for this node's parent
    model_matrix_ = node.GetParentModelMatrix();

    if (node.IsEnabled()) Render(node);
}

void GLRenderer::RenderEnvMaps(SceneObject& root) {
    for (auto kv : env_maps_) {
        auto scene_object = kv.first;
        EnvironmentMap* envmap = scene_object->GetComponent<EnvironmentMap>();

        SetMaterialOverride(envmap->RenderMaterial.Get());
        // Render backfaces only for shadowmaps
        if (envmap->RenderMaterial.Get() != nullptr && envmap->RenderMaterial.Get()->GetName() == "Depth Map Material") {
            glCullFace(GL_FRONT);
        }
        SetIgnoredNode(scene_object);

        int res = envmap->Resolution.Get();
        for (int i = 0; i < Cubemap::NUM_CUBEMAP_FACES; i++) {
            GLRenderableCubeMap& glcubemap = dynamic_cast<GLRenderableCubeMap&>(resource_manager_.GetGLTexture(envmap->GetCubemap()));
            glcubemap.BindFramebuffer(i);
            glViewport(0, 0, res, res);
            glClearColor(0.f,0.f,0.f,1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            RenderNode(root, envmap->GetViewMatrix(i, kv.second), envmap->GetProjection(), glm::vec2(res, res));
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glCullFace(GL_BACK);
    }
    SetIgnoredNode(nullptr);
}

#ifndef __APPLE__
void GLAPIENTRY GLErrorMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, const void* userParam ) {
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message );
}
#endif

// Render first the node, then recursively render its children
void GLRenderer::Render(SceneObject& node) {
    if (&node == ignored_node_) return;

    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Perform the transformation
    // NOTE: GLM uses column major ordering which is OpenGL's traditional layout
    model_matrix_ = model_matrix_ * node.GetTransform().GetMatrix();

    if (node_prefix_.empty() || node.GetName().compare(0, node_prefix_.length(), node_prefix_) == 0) {
        try {
            // If you have GL_INVALID_OPERATION errors, try uncommenting these:
            //glEnable              ( GL_DEBUG_OUTPUT );
            //glDebugMessageCallback( GLErrorMessageCallback, 0 );

            // Should start off with no errors
            GLCheckError();

            // Mesh rendering
            Geometry* geo = node.GetComponent<Geometry>();
            if (geo != nullptr) Render(node, *geo);

            // Particle System rendering
            ParticleSystem* particles = node.GetComponent<ParticleSystem>();
            if (particles != nullptr) Render(node, *particles);
            ConnectedParticleSystem* particles2 = node.GetComponent<ConnectedParticleSystem>();
            if (particles2 != nullptr) Render(node, *particles2);

            // Camera rendering
            if (draw_camera_) {
                Camera* rendercam = node.GetComponent<Camera>();
                if (rendercam != nullptr) Render(node, *rendercam);
            }

            // Lights rendering
            if (draw_lights_) {
                if (Light* light = node.GetComponent<Light>()) {
                    if (PointLight* pointlight = light->as<PointLight>()) Render(node, *pointlight);
                    else if (DirectionalLight* dirlight = light->as<DirectionalLight>()) Render(node, *dirlight);
                    else if (SpotLight* spotlight = light->as<SpotLight>()) Render(node, *spotlight);
                    else if (AreaLight* arealight = light->as<AreaLight>()) Render(node, *arealight);
                }
            }

            // Colliders rendering
            if (draw_colliders_) {
                SphereCollider* sphere = node.GetComponent<SphereCollider>();
                if (sphere != nullptr) Render(node, *sphere);

                PlaneCollider* plane = node.GetComponent<PlaneCollider>();
                if (plane != nullptr) Render(node, *plane);

                CylinderCollider* cylinder = node.GetComponent<CylinderCollider>();
                if (cylinder != nullptr) Render(node, *cylinder);
            }
        } catch (const RenderingException& e) {
            Debug::Log.WriteLine(e.what(), Priority::Error);
        }
    }

    // Render each child
    auto children = node.GetChildren();
    for (auto& child : children) {
        if (child->IsEnabled()) Render(*child);
    }

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject& node, PlaneCollider& collider) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Get rid of any scaling
    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat orientation; // Decompose returns the conjugate of the quaternion for some reason
    glm::decompose(model_matrix_, scale, orientation, translation, skew, perspective);
    model_matrix_ = glm::translate(glm::mat4(), translation);
    model_matrix_ = model_matrix_ * glm::toMat4(glm::conjugate(orientation));

    // Create a mesh to draw with
    Mesh* collider_mesh = asset_manager_->CreateMesh("__ColliderMesh__", MeshType::Lines);
    if (collider_mesh == nullptr) collider_mesh = asset_manager_->GetMesh("__ColliderMesh__");
    std::vector<float> points;
    float half_width = float(collider.Width.Get()) / 2.0f;
    float half_height = float(collider.Height.Get()) / 2.0f;
    // Top Edge
    points.push_back(-half_width); points.push_back(half_height); points.push_back(0);
    points.push_back(half_width); points.push_back(half_height); points.push_back(0);
    // Bot Edge
    points.push_back(-half_width); points.push_back(-half_height); points.push_back(0);
    points.push_back(half_width); points.push_back(-half_height); points.push_back(0);
    // Right Edge
    points.push_back(half_width); points.push_back(half_height); points.push_back(0);
    points.push_back(half_width); points.push_back(-half_height); points.push_back(0);
    // Left Edge
    points.push_back(-half_width); points.push_back(half_height); points.push_back(0);
    points.push_back(-half_width); points.push_back(-half_height); points.push_back(0);
    // Diagnol
    points.push_back(-half_width); points.push_back(half_height); points.push_back(0);
    points.push_back(half_width); points.push_back(-half_height); points.push_back(0);
    // Normal
    points.push_back(0); points.push_back(0); points.push_back(0);
    points.push_back(0); points.push_back(0); points.push_back(0.25f);
    collider_mesh->SetPositions(points);

    // Use Unlit Material to render with
    static const std::string COLLIDER_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(COLLIDER_MATERIAL);
    if (mat == nullptr) throw RenderingException(COLLIDER_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(COLLIDER_MATERIAL + " does not have a shader program.");
    mat->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.13f, 0.7f, 0.3f));

    // Get the GLResources and render the mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*collider_mesh).Render();

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject& node, SphereCollider& collider) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Get rid of any scaling
    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat orientation; // Decompose returns the conjugate of the quaternion for some reason
    glm::decompose(model_matrix_, scale, orientation, translation, skew, perspective);
    model_matrix_ = glm::translate(glm::mat4(), translation);
    model_matrix_ = model_matrix_ * glm::toMat4(glm::conjugate(orientation));

    // Create a mesh to draw with
    Mesh* collider_mesh = asset_manager_->CreateMesh("__ColliderMesh__", MeshType::Lines);
    if (collider_mesh == nullptr) collider_mesh = asset_manager_->GetMesh("__ColliderMesh__");
    std::vector<float> points;
    float radius = float(collider.Radius.Get());
    static const unsigned int subdivs = 100;
    static const float angle_step = 360.0f / subdivs;
    float angle = 0.0f;
    glm::vec2 prev_point(cosf(glm::radians(angle)) * radius, sinf(glm::radians(angle)) * radius);
    for (unsigned int i = 1; i <= subdivs; i++) {
        angle += angle_step;
        glm::vec2 curr_point(cosf(glm::radians(angle)) * radius, sinf(glm::radians(angle)) * radius);
        // YX-Circle
        points.push_back(0); points.push_back(prev_point.y); points.push_back(prev_point.x);
        points.push_back(0); points.push_back(curr_point.y); points.push_back(curr_point.x);
        // YZ-Circle
        points.push_back(prev_point.x); points.push_back(prev_point.y); points.push_back(0);
        points.push_back(curr_point.x); points.push_back(curr_point.y); points.push_back(0);
        // XZ-Circle
        points.push_back(prev_point.x); points.push_back(0); points.push_back(prev_point.y);
        points.push_back(curr_point.x); points.push_back(0); points.push_back(curr_point.y);
        prev_point = curr_point;
    }
    collider_mesh->SetPositions(points);

    // Use Unlit Material to render with
    static const std::string COLLIDER_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(COLLIDER_MATERIAL);
    if (mat == nullptr) throw RenderingException(COLLIDER_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(COLLIDER_MATERIAL + " does not have a shader program.");
    mat->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.13f, 0.7f, 0.3f));

    // Get the GLResources and render the mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*collider_mesh).Render();

    // Draw a final circle, facing the camera
    points.clear();
    angle = 0.0f;
    prev_point = glm::vec2(cosf(glm::radians(angle)) * radius, sinf(glm::radians(angle)) * radius);
    for (unsigned int i = 1; i <= subdivs; i++) {
        angle += angle_step;
        glm::vec2 curr_point(cosf(glm::radians(angle)) * radius, sinf(glm::radians(angle)) * radius);
        // YZ-Circle
        points.push_back(prev_point.x); points.push_back(prev_point.y); points.push_back(0);
        points.push_back(curr_point.x); points.push_back(curr_point.y); points.push_back(0);
        prev_point = curr_point;
    }
    collider_mesh->SetPositions(points);
    // Use the same rotation as the camera (get from the view_matrix)
    model_matrix_ = glm::translate(glm::mat4(), translation);
    glm::decompose(view_matrix_, scale, orientation, translation, skew, perspective);
    model_matrix_ = model_matrix_ * glm::toMat4(orientation);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*collider_mesh).Render();

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject& node, CylinderCollider& collider) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Get rid of any scaling
    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat orientation; // Decompose returns the conjugate of the quaternion for some reason
    glm::decompose(model_matrix_, scale, orientation, translation, skew, perspective);
    model_matrix_ = glm::translate(glm::mat4(), translation);
    model_matrix_ = model_matrix_ * glm::toMat4(glm::conjugate(orientation));

    // Create a mesh to draw with
    Mesh* collider_mesh = asset_manager_->CreateMesh("__ColliderMesh__", MeshType::Lines);
    if (collider_mesh == nullptr) collider_mesh = asset_manager_->GetMesh("__ColliderMesh__");
    std::vector<float> points;
    float radius = float(collider.Diameter.Get())*0.5f;
    float height = float(collider.Height.Get());
    static const unsigned int subdivs = 18;
    static const unsigned int stacks = 10;
    static const float angle_step = 360.0f / subdivs;
    // draw stacked circles
    for (unsigned int s = 0; s <= stacks; s++) {
        float angle = 0.0f;
        glm::vec2 prev_point(cosf(glm::radians(angle)) * radius, sinf(glm::radians(angle)) * radius);
        float h = height * ((s * 1.0f/ stacks) - 0.5f);
        for (unsigned int i = 1; i <= subdivs; i++) {
            angle += angle_step;
            glm::vec2 curr_point(cosf(glm::radians(angle)) * radius, sinf(glm::radians(angle)) * radius);
            // ZX-Circle
            points.push_back(prev_point.y); points.push_back(h); points.push_back(prev_point.x);
            points.push_back(curr_point.y); points.push_back(h); points.push_back(curr_point.x);
            prev_point = curr_point;
        }
    }
    // draw vertical lines
    float angle = 0.0f;
    for (unsigned int i = 0; i < subdivs; i++) {
        glm::vec2 curr_point(cosf(glm::radians(angle)) * radius, sinf(glm::radians(angle)) * radius);
        // top cap to periphery
        points.push_back(0); points.push_back(0.5f * height); points.push_back(0);
        points.push_back(curr_point.y); points.push_back(0.5f * height); points.push_back(curr_point.x);
        // down along periphery
        points.push_back(curr_point.y); points.push_back(0.5f * height); points.push_back(curr_point.x);
        points.push_back(curr_point.y); points.push_back(-0.5f * height); points.push_back(curr_point.x);
        // periphery to bottom cap
        points.push_back(curr_point.y); points.push_back(-0.5f * height); points.push_back(curr_point.x);
        points.push_back(0); points.push_back(-0.5f * height); points.push_back(0);

        angle += angle_step;
    }

    collider_mesh->SetPositions(points);

    // Use Unlit Material to render with
    static const std::string COLLIDER_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(COLLIDER_MATERIAL);
    if (mat == nullptr) throw RenderingException(COLLIDER_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(COLLIDER_MATERIAL + " does not have a shader program.");
    mat->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.13f, 0.7f, 0.3f));

    // Get the GLResources and render the mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*collider_mesh).Render();

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject& node, PointLight& light) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Get rid of any scaling
    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat orientation; // Decompose returns the conjugate of the quaternion for some reason
    glm::decompose(model_matrix_, scale, orientation, translation, skew, perspective);
    model_matrix_ = glm::translate(glm::mat4(), translation);
    model_matrix_ = model_matrix_ * glm::toMat4(glm::conjugate(orientation));

    // Create a mesh to draw with
    Mesh* light_mesh = asset_manager_->CreateMesh("__LightMesh__", MeshType::Lines);
    if (light_mesh == nullptr) light_mesh = asset_manager_->GetMesh("__LightMesh__");
    std::vector<float> points;
    static const float outer_radius = 0.2f;
    static const float smaller_outer_radius = outer_radius / 1.25f;
    static const float inner_radius = 0.05f;
    static const float gap = inner_radius;
    static const unsigned int subdivs = 40;
    static const float angle_step = 360.0f / subdivs;
    float angle = 0.0f;
    glm::vec2 prev_point(cosf(glm::radians(angle)) * inner_radius, sinf(glm::radians(angle)) * inner_radius);
    for (unsigned int i = 1; i <= subdivs; i++) {
        angle += angle_step;
        glm::vec2 curr_point(cosf(glm::radians(angle)) * inner_radius, sinf(glm::radians(angle)) * inner_radius);
        // YX-Circle
        points.push_back(0); points.push_back(prev_point.y); points.push_back(prev_point.x);
        points.push_back(0); points.push_back(curr_point.y); points.push_back(curr_point.x);
        // YZ-Circle
        points.push_back(prev_point.x); points.push_back(prev_point.y); points.push_back(0);
        points.push_back(curr_point.x); points.push_back(curr_point.y); points.push_back(0);
        // XZ-Circle
        points.push_back(prev_point.x); points.push_back(0); points.push_back(prev_point.y);
        points.push_back(curr_point.x); points.push_back(0); points.push_back(curr_point.y);
        prev_point = curr_point;
    }
    // X-Axis
    points.push_back(inner_radius + gap); points.push_back(0); points.push_back(0);
    points.push_back(outer_radius); points.push_back(0); points.push_back(0);
    points.push_back(-inner_radius - gap); points.push_back(0); points.push_back(0);
    points.push_back(-outer_radius); points.push_back(0); points.push_back(0);
    // Y-Axis
    points.push_back(0); points.push_back(inner_radius + gap); points.push_back(0);
    points.push_back(0); points.push_back(outer_radius); points.push_back(0);
    points.push_back(0); points.push_back(-inner_radius - gap); points.push_back(0);
    points.push_back(0); points.push_back(-outer_radius); points.push_back(0);
    // Z-Axis
    points.push_back(0); points.push_back(0); points.push_back(inner_radius + gap);
    points.push_back(0); points.push_back(0); points.push_back(outer_radius);
    points.push_back(0); points.push_back(0); points.push_back(-inner_radius - gap);
    points.push_back(0); points.push_back(0); points.push_back(-outer_radius);
    // YX-Plane
    points.push_back(0); points.push_back(cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(sinf(glm::radians(45.0f)) * (inner_radius + gap));
    points.push_back(0); points.push_back(cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(sinf(glm::radians(45.0f)) * smaller_outer_radius);
    points.push_back(0); points.push_back(cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(-sinf(glm::radians(45.0f)) * (inner_radius + gap));
    points.push_back(0); points.push_back(cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(-sinf(glm::radians(45.0f)) * smaller_outer_radius);
    points.push_back(0); points.push_back(-cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(sinf(glm::radians(45.0f)) * (inner_radius + gap));
    points.push_back(0); points.push_back(-cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(sinf(glm::radians(45.0f)) * smaller_outer_radius);
    points.push_back(0); points.push_back(-cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(-sinf(glm::radians(45.0f)) * (inner_radius + gap));
    points.push_back(0); points.push_back(-cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(-sinf(glm::radians(45.0f)) * smaller_outer_radius);
    // YZ-Plane
    points.push_back(cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(sinf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(0);
    points.push_back(cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(sinf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(0);
    points.push_back(cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(-sinf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(0);
    points.push_back(cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(-sinf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(0);
    points.push_back(-cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(sinf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(0);
    points.push_back(-cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(sinf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(0);
    points.push_back(-cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(-sinf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(0);
    points.push_back(-cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(-sinf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(0);
    // XZ-Plane
    points.push_back(cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(0); points.push_back(sinf(glm::radians(45.0f)) * (inner_radius + gap));
    points.push_back(cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(0); points.push_back(sinf(glm::radians(45.0f)) * smaller_outer_radius);
    points.push_back(cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(0); points.push_back(-sinf(glm::radians(45.0f)) * (inner_radius + gap));
    points.push_back(cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(0); points.push_back(-sinf(glm::radians(45.0f)) * smaller_outer_radius);
    points.push_back(-cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(0); points.push_back(sinf(glm::radians(45.0f)) * (inner_radius + gap));
    points.push_back(-cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(0); points.push_back(sinf(glm::radians(45.0f)) * smaller_outer_radius);
    points.push_back(-cosf(glm::radians(45.0f)) * (inner_radius + gap)); points.push_back(0); points.push_back(-sinf(glm::radians(45.0f)) * (inner_radius + gap));
    points.push_back(-cosf(glm::radians(45.0f)) * smaller_outer_radius); points.push_back(0); points.push_back(-sinf(glm::radians(45.0f)) * smaller_outer_radius);
    light_mesh->SetPositions(points);

    // Use Unlit Material to render with
    static const std::string LIGHT_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(LIGHT_MATERIAL);
    if (mat == nullptr) throw RenderingException(LIGHT_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(LIGHT_MATERIAL + " does not have a shader program.");
    // Set color to be the diffuse component of the light
    mat->Uniforms.Get<ColorProperty>("Color")->Set(light.GetIntensity());

    // Get the GLResources and render the mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*light_mesh).Render();

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject& node, DirectionalLight& light) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Create a mesh to draw with
    Mesh* light_mesh = asset_manager_->CreateMesh("__LightMesh__", MeshType::Lines);
    if (light_mesh == nullptr) light_mesh = asset_manager_->GetMesh("__LightMesh__");
    std::vector<float> points;
    // Shaft
    points.push_back(0); points.push_back(0.25f); points.push_back(0);
    points.push_back(0); points.push_back(-0.25f); points.push_back(0);
    // Arrowhead
    points.push_back(0); points.push_back(-0.35f); points.push_back(0);
    points.push_back(0.1f); points.push_back(-0.25f); points.push_back(0);
    points.push_back(0); points.push_back(-0.35f); points.push_back(0);
    points.push_back(-0.1f); points.push_back(-0.25f); points.push_back(0);
    points.push_back(0.1f); points.push_back(-0.25f); points.push_back(0);
    points.push_back(-0.1f); points.push_back(-0.25f); points.push_back(0);
    light_mesh->SetPositions(points);

    // Use Unlit Material to render with
    static const std::string LIGHT_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(LIGHT_MATERIAL);
    if (mat == nullptr) throw RenderingException(LIGHT_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(LIGHT_MATERIAL + " does not have a shader program.");
    // Set color to be the diffuse component of the light
    mat->Uniforms.Get<ColorProperty>("Color")->Set(light.GetIntensity());

    // Get the GLResources and render the mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);

    // Draw center arrow
    model_matrix_ = glm::rotate(matrix_stack_.top(), glm::radians(90.0f), glm::vec3(0, 1, 0));
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*light_mesh).Render();

    // Draw three arrows encircling the center arrow
    model_matrix_ = glm::translate(matrix_stack_.top(), glm::vec3(0.0f, 0.0f, -0.2f));
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*light_mesh).Render();

    model_matrix_ = glm::translate(matrix_stack_.top(), glm::vec3(cosf(glm::radians(30.0f)) * 0.2f, 0.0f, sinf(glm::radians(30.0f)) * 0.2f));
    model_matrix_ = glm::rotate(model_matrix_, glm::radians(60.0f), glm::vec3(0, 1, 0));
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*light_mesh).Render();

    model_matrix_ = glm::translate(matrix_stack_.top(), glm::vec3(-cosf(glm::radians(30.0f)) * 0.2f, 0.0f, sinf(glm::radians(30.0f)) * 0.2f));
    model_matrix_ = glm::rotate(model_matrix_, glm::radians(-60.0f), glm::vec3(0, 1, 0));
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*light_mesh).Render();

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject &node, SpotLight& light) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Create a mesh to draw with
    Mesh* light_mesh = asset_manager_->CreateMesh("__LightMesh__", MeshType::Lines);
    if (light_mesh == nullptr) light_mesh = asset_manager_->GetMesh("__LightMesh__");
    std::vector<float> points;
    static const float outer_radius = 0.3f;
    static const float inner_radius = 0.05f;
    static const unsigned int subdivs = 40;
    static const float angle_step = 360.0f / subdivs;
    float angle = 0.0f;
    float cutoff_angle = 60.0 / 180.0 * M_PI;

    glm::vec2 prev_point(cosf(glm::radians(angle)) * inner_radius, sinf(glm::radians(angle)) * inner_radius);
    glm::vec2 outer_prev_point(-sinf(cutoff_angle / 2) * sinf(glm::radians(angle)), cosf(glm::radians(angle)) * sinf(cutoff_angle / 2) * outer_radius);
    for (unsigned int i = 1; i <= subdivs; i++) {
        angle += angle_step;
        glm::vec2 curr_point(cosf(glm::radians(angle)) * inner_radius, sinf(glm::radians(angle)) * inner_radius);
        glm::vec2 outer_curr_point(-sinf(glm::radians(angle)) * sinf(cutoff_angle / 2) * outer_radius, cosf(glm::radians(angle)) * sinf(cutoff_angle / 2) * outer_radius);
        // YX-Circle
        points.push_back(0); points.push_back(prev_point.y); points.push_back(prev_point.x);
        points.push_back(0); points.push_back(curr_point.y); points.push_back(curr_point.x);
        // YZ-Circle
        points.push_back(prev_point.x); points.push_back(prev_point.y); points.push_back(0);
        points.push_back(curr_point.x); points.push_back(curr_point.y); points.push_back(0);
        // XZ-Circle
        points.push_back(prev_point.x); points.push_back(0); points.push_back(prev_point.y);
        points.push_back(curr_point.x); points.push_back(0); points.push_back(curr_point.y);
        // outer YX-Circle
        points.push_back(outer_prev_point.x); points.push_back(-outer_radius + 0.05f); points.push_back(outer_prev_point.y);
        points.push_back(outer_curr_point.x); points.push_back(-outer_radius + 0.05f); points.push_back(outer_curr_point.y);
        outer_prev_point = outer_curr_point;
        prev_point = curr_point;
    }
    // Y-Axis - direction of light
    points.push_back(0); points.push_back(0); points.push_back(0);
    points.push_back(0); points.push_back(-outer_radius*1.1); points.push_back(0);

    float p1_x = inner_radius;
    float p1_y = 0;
    float p2_x = sinf(cutoff_angle / 2) * outer_radius;
    float p2_y = cosf(cutoff_angle / 2) * outer_radius;

    points.push_back(p1_x); points.push_back(-p1_y); points.push_back(0);
    points.push_back(p2_x); points.push_back(-p2_y); points.push_back(0);

    for (float i = 60.0f; i < 360.0f; i += 60.0f) {
        points.push_back(cosf(glm::radians(i)) * p1_x); points.push_back(-p1_y); points.push_back(sinf(glm::radians(i)) * p1_x);
        points.push_back(cosf(glm::radians(i)) * p2_x); points.push_back(-p2_y); points.push_back(sinf(glm::radians(i)) * p2_x);
    }

    light_mesh->SetPositions(points);

    // Use Unlit Material to render with
    static const std::string LIGHT_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(LIGHT_MATERIAL);
    if (mat == nullptr) throw RenderingException(LIGHT_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(LIGHT_MATERIAL + " does not have a shader program.");
    // Set color to be the diffuse component of the light
    mat->Uniforms.Get<ColorProperty>("Color")->Set(light.GetIntensity());

    // Get the GLResources and render the mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*light_mesh).Render();

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject& node, AreaLight& light) {
    // Create a mesh to draw with
    Mesh* arealight_mesh = asset_manager_->CreateMesh("__AreaLightMesh__", MeshType::Lines);
    if (arealight_mesh == nullptr) {
        arealight_mesh = asset_manager_->GetMesh("__AreaLightMesh__");
        std::vector<float> points;
        // Top Edge
        points.push_back(-0.5f); points.push_back(0); points.push_back(0.5f);
        points.push_back(0.5f); points.push_back(0); points.push_back(0.5f);
        // Bot Edge
        points.push_back(-0.5f); points.push_back(0); points.push_back(-0.5f);
        points.push_back(0.5f); points.push_back(0); points.push_back(-0.5f);
        // Right Edge
        points.push_back(0.5f); points.push_back(0); points.push_back(0.5f);
        points.push_back(0.5f); points.push_back(0); points.push_back(-0.5f);
        // Left Edge
        points.push_back(-0.5f); points.push_back(0); points.push_back(0.5f);
        points.push_back(-0.5f); points.push_back(0); points.push_back(-0.5f);
        // Diagnol
        points.push_back(-0.5f); points.push_back(0); points.push_back(0.5f);
        points.push_back(0.5f); points.push_back(0); points.push_back(-0.5f);
        // Normal
        points.push_back(0); points.push_back(0); points.push_back(0);
        points.push_back(0); points.push_back(0.25f); points.push_back(0);
        arealight_mesh->SetPositions(points);
    }

    // Use Unlit Material to render with
    static const std::string COLLIDER_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(COLLIDER_MATERIAL);
    if (mat == nullptr) throw RenderingException(COLLIDER_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(COLLIDER_MATERIAL + " does not have a shader program.");
    mat->Uniforms.Get<ColorProperty>("Color")->Set(light.Color.GetRGB());

    // Get the GLResources and render the mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*arealight_mesh).Render();
}

static std::unordered_map<RayType, glm::vec3, EnumClassHash> DebugRayColors = {
    {RayType::camera, {1.0f,0.0f,0.0f}},
    {RayType::reflection, {0.0f, 0.7f, 0.9f}},
    {RayType::diffuse_reflection, {0.1f, 0.5f, 0.5f}},
    {RayType::refraction, {1.0f, 0.8f, 0.0f}},
    {RayType::shadow, {0.70f, 0.45f, 0.82f}},
    {RayType::hit_normal, {0.5f, 1.0f, 0.5f}}
};

void GLRenderer::Render(SceneObject& node, Camera& rendercam) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Get rid of any scaling
    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat orientation; // Decompose returns the conjugate of the quaternion for some reason
    glm::decompose(model_matrix_, scale, orientation, translation, skew, perspective);
    model_matrix_ = glm::translate(glm::mat4(), translation);
    model_matrix_ = model_matrix_ * glm::toMat4(glm::conjugate(orientation));

    // Create a mesh to draw with
    Mesh* camera_mesh = asset_manager_->CreateMesh("__CameraMesh__", MeshType::Lines);
    if (camera_mesh == nullptr) camera_mesh = asset_manager_->GetMesh("__CameraMesh__");
    glm::vec3 look(0, 0, -1);
    float height = 1.0f;
    float width = height * float(rendercam.GetAspectRatio());
    float half_height = height / 2.0f;
    float half_width = width / 2.0f;
    std::vector<float> points;
    // Top Left
    points.push_back(0); points.push_back(0); points.push_back(0);
    points.push_back(look.x - half_width); points.push_back(look.y + half_height); points.push_back(look.z);
    // Top Edge
    points.push_back(look.x - half_width); points.push_back(look.y + half_height); points.push_back(look.z);
    points.push_back(look.x + half_width); points.push_back(look.y + half_height); points.push_back(look.z);
    // Top Right
    points.push_back(0); points.push_back(0); points.push_back(0);
    points.push_back(look.x + half_width); points.push_back(look.y + half_height); points.push_back(look.z);
    // Bot Left
    points.push_back(0); points.push_back(0); points.push_back(0);
    points.push_back(look.x - half_width); points.push_back(look.y - half_height); points.push_back(look.z);
    // Bot Edge
    points.push_back(look.x - half_width); points.push_back(look.y - half_height); points.push_back(look.z);
    points.push_back(look.x + half_width); points.push_back(look.y - half_height); points.push_back(look.z);
    // Bot Right
    points.push_back(0); points.push_back(0); points.push_back(0);
    points.push_back(look.x + half_width); points.push_back(look.y - half_height); points.push_back(look.z);
    // Left Edge
    points.push_back(look.x - half_width); points.push_back(look.y + half_height); points.push_back(look.z);
    points.push_back(look.x - half_width); points.push_back(look.y - half_height); points.push_back(look.z);
    // Right Edge
    points.push_back(look.x + half_width); points.push_back(look.y + half_height); points.push_back(look.z);
    points.push_back(look.x + half_width); points.push_back(look.y - half_height); points.push_back(look.z);
    camera_mesh->SetPositions(points);

    // Use Unlit Material to render with
    static const std::string CAMERA_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(CAMERA_MATERIAL);
    if (mat == nullptr) throw RenderingException(CAMERA_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(CAMERA_MATERIAL + " does not have a shader program.");
    // Set color to be white
    mat->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(1.0f, 1.0f, 1.0f));

    // Get the GLResources and render the camera mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*camera_mesh).Render();

    if (rendercam.debug_rays_.size() > 0) {
        model_matrix_ = glm::mat4(); //set the model matrix to identity because the rays are in world space
        for(auto p : DebugRayColors) {
            RayType rtype = p.first;

            if (rendercam.trace_debug_views.find((int)rtype) != rendercam.trace_debug_views.end()) {
                if (!(rendercam.trace_debug_views[(int)rtype]->Get())) {
                    continue;
                }
            }

            // Create a mesh to draw with
            Mesh* rays_mesh = asset_manager_->CreateMesh("__RayDebugMesh__", MeshType::Triangles);
            if (rays_mesh == nullptr) rays_mesh = asset_manager_->GetMesh("__RayDebugMesh__");
            std::vector<float> points;
            std::vector<unsigned int> ids;
            int baseID = 0;
            for(auto& r : rendercam.debug_rays_) {
                if (r.type == rtype) {
                    glm::vec3 dir = r.p1 - r.p2;
                    glm::vec3 tan1 = glm::cross(dir, glm::vec3(1,0,0));
                    if (glm::length(tan1) < 0.1) {
                        tan1 = glm::cross(dir, glm::vec3(0,1,0));
                    }
                    tan1 = glm::normalize(tan1);
                    glm::vec3 tan2 = glm::normalize(glm::cross(dir, tan1));

                    int steps=8;
                    float rad = 0.008;
                    float point = 0.01;

                    baseID = points.size()/3;

                    for (int i=0; i<steps; i++) {
                        glm::vec3 ofs = rad * (cosf(glm::radians((double)i*360.0/steps)) * tan1 + sinf(glm::radians((double)i*360.0/steps)) * tan2);
                        glm::vec3 l1 = r.p1 + ofs;
                        glm::vec3 l2 = r.p2 + ofs;
                        points.push_back(l1.x); points.push_back(l1.y); points.push_back(l1.z);
                        points.push_back(l2.x); points.push_back(l2.y); points.push_back(l2.z);
                        ids.push_back(baseID+i*2); ids.push_back(baseID+i*2 +1); ids.push_back(baseID+((i+1)%steps)*2);
                        ids.push_back(baseID+((i+1)%steps)*2+1); ids.push_back(baseID+((i+1)%steps)*2); ids.push_back(baseID+i*2 +1);
                        ids.push_back(baseID+i*2); ids.push_back(baseID+((i+1)%steps)*2); ids.push_back(baseID+steps*2);
                        ids.push_back(baseID+((i+1)%steps)*2+1);  ids.push_back(baseID+i*2 +1); ids.push_back(baseID+steps*2 + 1);
                    }

                    glm::vec3 po = r.p1 - point*glm::normalize(r.p2-r.p1);

                    points.push_back(po.x); points.push_back(po.y); points.push_back(po.z);
                    po = r.p2 + point*glm::normalize(r.p2-r.p1);
                    points.push_back(po.x); points.push_back(po.y); points.push_back(po.z);

                }
            }
            rays_mesh->SetPositions(points);
            rays_mesh->SetTriangles(ids);

            // Use Unlit Material to render with
            static const std::string RAYS_MATERIAL = "_internal Unlit Material";
            Material* mat = asset_manager_->GetMaterial(RAYS_MATERIAL);
            if (mat == nullptr) throw RenderingException(RAYS_MATERIAL + " does not exist.");
            ShaderProgram* program = mat->Shader.Get();
            if (program == nullptr) throw RenderingException(RAYS_MATERIAL + " does not have a shader program.");
            // Set color to be white
            mat->Uniforms.Get<ColorProperty>("Color")->Set(p.second);

            // Get the GLResources and render the camera mesh
            GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
            SetUniforms(shader, *mat, node);
            resource_manager_.GetGLMesh(*rays_mesh).Render();
        }
    }

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject& node, Geometry &geo) {
    if (node.GetComponent<ConnectedParticleSystem>() != nullptr)
        return;

    // Get the mesh to render
    Mesh* mesh = rendering_selection_ ? geo.GetEditorMesh() : geo.GetRenderMesh();
    if (mesh == nullptr) throw RenderingException(node.GetName() + "'s Geometry does not have a Mesh");
    // Get the material of the mesh
    Material* material = geo.RenderMaterial.Get();
    if (material_override_ != nullptr) {
        // Any use of material overrides should ignore wire meshes
        if (mesh->GetMeshType() == MeshType::Lines) return;

        if (!node.IsInternal()) material = material_override_;
        // TODO: Better way to do this than using _mouse and _gizmo?
        if (material_override_->GetName().compare(0,6,"_mouse") == 0) {
            // Gizmos are clickable (even though they are internal)
            if (node.GetName().compare(0,6,"_gizmo") == 0) material = material_override_;
        }
    }
    if (material == nullptr) {
        if (node.GetComponent<ParticleSystem>() == nullptr/* && node.GetComponent<ConnectedParticleSystem>() == nullptr*/)
            throw RenderingException(node.GetName() + "'s Mesh Renderer material does not have a shader program");
        else return;
    }
    // TODO: Batch rendering
    // Pick the shader to use
    ShaderProgram* program = material->Shader.Get();
    if (program == nullptr) throw RenderingException(node.GetName() + " does not have a suitable OpenGLShaderProgram");
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *material, node);

    if (material->GetName() == "Alpha Test Material")
        glDisable(GL_CULL_FACE);

    GLMesh& gl_mesh = resource_manager_.GetGLMesh(*mesh);
    gl_mesh.Render();

    glEnable(GL_CULL_FACE);
}

void GLRenderer::Render(SceneObject& node, ParticleSystem& particles) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Get the mesh to render as a Particle
    Geometry* geo = node.GetComponent<Geometry>();
    if (geo == nullptr) throw RenderingException(node.GetName() + " does not have a Geometry component to render");
    Mesh* mesh = geo->GetRenderMesh();
    if (mesh == nullptr) throw RenderingException(node.GetName() + "'s Geometry does not have a Mesh");

    // Get the material of the mesh
    Material* material = particles.ParticleMaterial.Get();
    if (material == nullptr) throw RenderingException(node.GetName() + "'s Mesh Renderer material does not have a shader program");

    // Get the shader to use
    ShaderProgram* program = material->Shader.Get();
    if (program == nullptr) throw RenderingException(node.GetName() + " does not have a suitable OpenGLShaderProgram");
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);

    // Get the parent rotation to rotate the orientation of the particle with the emitter
    glm::vec3 s, t, skew;
    glm::vec4 perspective;
    glm::quat orientation; // Decompose returns the conjugate of the quaternion for some reason
    glm::decompose(model_matrix_, s, orientation, t, skew, perspective);
    glm::mat4 parent_rot = glm::toMat4(glm::conjugate(orientation));
    //
    // EXTRA CREDIT:
    // Billboard the particles when billboarding is enabled (rotate so they always face the camera)
    // view_matrix_ will be useful for this

    // For each particle:
    for (auto& particle : particles.GetParticles()) {
        // Set the model_matrix to reflect the particle's world position and rotation
        glm::mat4 translation = glm::translate(glm::mat4(), particle->Position);
        // ZXY Rotation
        glm::quat QuatAroundX = glm::angleAxis( glm::radians(particle->Rotation.x), glm::vec3(1.0, 0.0, 0.0) );
        glm::quat QuatAroundY = glm::angleAxis( glm::radians(particle->Rotation.y), glm::vec3(0.0, 1.0, 0.0) );
        glm::quat QuatAroundZ = glm::angleAxis( glm::radians(particle->Rotation.z), glm::vec3(0.0, 0.0, 1.0) );
        glm::mat4 rotation = glm::toMat4(QuatAroundZ * QuatAroundY * QuatAroundX);
        model_matrix_ = translation * parent_rot * rotation;

        // Pass the model_matrix and other uniforms to the shader
        SetUniforms(shader, *material, node);
        // TODO: Instanced rendering, for now just draw glDrawElements once for each particle
        resource_manager_.GetGLMesh(*mesh).Render();
    }

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();
}

void GLRenderer::Render(SceneObject& node, ConnectedParticleSystem& particles) {
    // Push onto the matrix stack
    matrix_stack_.push(model_matrix_);

    // Get the mesh to render as a Particle
    Geometry* geo = node.GetComponent<Geometry>();
    if (geo == nullptr) throw RenderingException(node.GetName() + " does not have a Geometry component to render");
    Mesh* mesh = geo->GetRenderMesh();
    if (mesh == nullptr) throw RenderingException(node.GetName() + "'s Geometry does not have a Mesh");

    // Get the material of the mesh
    Material* material = particles.ParticleMaterial.Get();
    if (material == nullptr) throw RenderingException(node.GetName() + "'s Mesh Renderer material does not have a shader program");

    // Get the shader to use
    ShaderProgram* program = material->Shader.Get();
    if (program == nullptr) throw RenderingException(node.GetName() + " does not have a suitable OpenGLShaderProgram");
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);

    // Store the particle positions to use as mesh vertices
    std::vector<glm::vec3> vert_pos = std::vector<glm::vec3>();

    // For each particle:
    for (auto& particle : particles.GetParticles()) {
        vert_pos.push_back(particle->Position);

        if (particles.ShowParticles()) {
            // Set the model_matrix to reflect the particle's world position and rotation
            glm::mat4 translation = glm::translate(glm::mat4(), particle->Position);
            // ZXY Rotation
            glm::quat QuatAroundX = glm::angleAxis( glm::radians(particle->Rotation.x), glm::vec3(1.0, 0.0, 0.0) );
            glm::quat QuatAroundY = glm::angleAxis( glm::radians(particle->Rotation.y), glm::vec3(0.0, 1.0, 0.0) );
            glm::quat QuatAroundZ = glm::angleAxis( glm::radians(particle->Rotation.z), glm::vec3(0.0, 0.0, 1.0) );
            glm::mat4 rotation = glm::toMat4(QuatAroundZ * QuatAroundY * QuatAroundX);
            model_matrix_ = translation * rotation;
            // Pass the model_matrix and other uniforms to the shader
            SetUniforms(shader, *material, node);
            // TODO: Instanced rendering, for now just draw glDrawElements once for each particle
            resource_manager_.GetGLMesh(*mesh).Render();
        }
    }
    if (vert_pos.size() > 0)
        RenderDeformedMesh(node, vert_pos);

    // Pop from the matrix stack
    model_matrix_ = matrix_stack_.top();
    matrix_stack_.pop();

}

void GLRenderer::RenderDeformedMesh(SceneObject& node, std::vector<glm::vec3> vert_pos) {
    // This does:
    // For each vertex, render a line between itself and every other vertex.
    // You may change this code to render your deformed mesh differently.

    std::vector<float> points;
    model_matrix_ = glm::mat4(1);
    for (int i = 0; i < vert_pos.size(); i++) {
        for (int j = i + 1; j < vert_pos.size(); j++) {
            // X, Y, Z for first point
            points.push_back(vert_pos[i].x);
            points.push_back(vert_pos[i].y);
            points.push_back(vert_pos[i].z);
            // X, Y, Z for second point
            points.push_back(vert_pos[j].x);
            points.push_back(vert_pos[j].y);
            points.push_back(vert_pos[j].z);
        }
    }

    // Create a mesh to draw with
    Mesh* deformed_mesh = asset_manager_->CreateMesh("__DeformedCube__", MeshType::Lines);
    if (deformed_mesh == nullptr) deformed_mesh = asset_manager_->GetMesh("__DeformedCube__");

    deformed_mesh->SetPositions(points);

    // Use Unlit Material to render with
    static const std::string DEFORMED_MATERIAL = "_internal Unlit Material";
    Material* mat = asset_manager_->GetMaterial(DEFORMED_MATERIAL);
    if (mat == nullptr) throw RenderingException(DEFORMED_MATERIAL + " does not exist.");
    ShaderProgram* program = mat->Shader.Get();
    if (program == nullptr) throw RenderingException(DEFORMED_MATERIAL + " does not have a shader program.");
    mat->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.13f, 0.7f, 0.3f));

    // Get the GLResources and render the mesh
    GLShaderProgram& shader = resource_manager_.GetGLShaderProgram(*program);
    SetUniforms(shader, *mat, node);
    resource_manager_.GetGLMesh(*deformed_mesh).Render();
}

void GLRenderer::SetUniforms(GLShaderProgram& shader, Material& material, SceneObject& node) {
    GLuint shader_program = shader.GetProgram();
    glUseProgram(shader_program);

    // Loop through all the ShaderProgram's Uniforms and set them appropriately.
    // We have to do this everytime since ShaderPrograms can be shared between different materials.
    std::map<std::string, std::pair<GLint, DataType>> uniforms = shader.GetUniformLocations();
    GLuint tex_counter = 1; //there are issues when starting at zero
    for (auto& uniform : uniforms) {
        // Query for the uniform's location
        GLint uniform_loc = uniform.second.first;
        assert(uniform_loc >= 0);

        // If the uniform is a builtin variable, we need to set it
        auto builtin_uniforms = GLShaderProgram::BuiltinUniforms();
        if (builtin_uniforms.find(uniform.first) != builtin_uniforms.end()) {
            if (uniform.first == "model_matrix") {
                glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(model_matrix_));
            } else if (uniform.first == "view_matrix") {
                glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(view_matrix_));
            } else if (uniform.first == "projection_matrix") {
                glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(proj_matrix_));
            } else if (uniform.first == "screen_width") {
                // TODO: Should be an int
                glUniform1f(uniform_loc, screen_size_.x);
            } else if (uniform.first == "screen_height") {
                glUniform1f(uniform_loc, screen_size_.y);
            } else if (uniform.first == "object_id") {
                glUniform1i(uniform_loc, node.GetUID());
            } else if (uniform.first == "environment_map") {
                EnvironmentMap* envmap = node.GetComponent<EnvironmentMap>();
                if (envmap) {
                    glUniform1i(uniform_loc, tex_counter);
                    resource_manager_.GetGLTexture(envmap->GetCubemap()).Bind(GL_TEXTURE0 + tex_counter);
                    tex_counter++;
                }
            }
            // Set the Lights later
        } else {
            // Otherwise we can just pass on the value that we assume has been set by the user on the material
            Property* prop = material.Uniforms.GetProperty(uniform.first);
            assert(prop != nullptr); // Should never be true
            switch(uniform.second.second) {
                case DataType::Float: {
                    DoubleProperty* float_prop = dynamic_cast<DoubleProperty*>(prop);
                    assert(float_prop != nullptr);
                    glUniform1f(uniform_loc, float_prop->Get());
                    break; }
                case DataType::Double: {
                    DoubleProperty* dbl_prop = dynamic_cast<DoubleProperty*>(prop);
                    assert(dbl_prop != nullptr);
                    glUniform1d(uniform_loc, dbl_prop->Get());
                    break; }
                case DataType::Float3: {
                    Vec3Property* vec3_prop = dynamic_cast<Vec3Property*>(prop);
                    assert(vec3_prop != nullptr);
                    glm::vec3 v = vec3_prop->Get();
                    glUniform3f(uniform_loc, v.x, v.y, v.z);
                    break; }
                case DataType::Double3: {
                    Vec3Property* vec3_prop = dynamic_cast<Vec3Property*>(prop);
                    assert(vec3_prop != nullptr);
                    glm::vec3 v = vec3_prop->Get();
                    glUniform3d(uniform_loc, v.x, v.y, v.z);
                    break; }
                case DataType::UInt:
                case DataType::Int: {
                    IntProperty* int_prop = dynamic_cast<IntProperty*>(prop);
                    glUniform1i(uniform_loc, int_prop->Get());
                    break; }
                case DataType::Float4:
                case DataType::Double4:
                case DataType::FloatMat4x4:
                case DataType::DoubleMat4x4:
                    assert(false); // Unimplemented
                    break;
                case DataType::Bool: {
                    BooleanProperty* bool_prop = dynamic_cast<BooleanProperty*>(prop);
                    assert(bool_prop != nullptr);
                    glUniform1i(uniform_loc, bool_prop->Get());
                    break; }
                case DataType::Texture2D: {
                    TextureProperty* tex_prop = dynamic_cast<TextureProperty*>(prop);
                    assert(tex_prop != nullptr);
                    auto tex = tex_prop->Get();
                    glUniform1i(uniform_loc, tex_counter);
                    if (tex != nullptr) {
                        resource_manager_.GetGLTexture(*tex).Bind(GL_TEXTURE0 + tex_counter);
                    } else if (default_texture_ != nullptr) {
                        resource_manager_.GetGLTexture(*default_texture_).Bind(GL_TEXTURE0 + tex_counter);
                    }
                    tex_counter++;
                    break; }
                case DataType::ColorRGB: {
                    ColorProperty* color_prop = dynamic_cast<ColorProperty*>(prop);
                    assert(color_prop != nullptr);
                    glUniform3fv(uniform_loc, 1, glm::value_ptr(color_prop->Get()));
                    break; }
                case DataType::ColorRGBA: {
                    ColorProperty* color_prop = dynamic_cast<ColorProperty*>(prop);
                    assert(color_prop != nullptr);
                    glUniform4fv(uniform_loc, 1, glm::value_ptr(color_prop->Get()));
                    break; }
                case DataType::Cubemap: {
                    ResourceProperty<Cubemap>* cubemap_prop = dynamic_cast<ResourceProperty<Cubemap>*>(prop);
                    assert(cubemap_prop != nullptr);
                    auto cubemap = cubemap_prop->Get();
                    glUniform1i(uniform_loc, tex_counter);
                    if (cubemap != nullptr) {
                        resource_manager_.GetGLTexture(*cubemap).Bind(GL_TEXTURE0 + tex_counter);
                    } else {
                        glActiveTexture(GL_TEXTURE0 + tex_counter);
                        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    }
                    tex_counter++;
                    break; }
                case DataType::Unsupported:
                    break;
            }
        }
    }

    // Set the Builtin Light Uniforms
    const unsigned int NUM_LIGHTS = 4;
    float* ambient = new float[NUM_LIGHTS * 3];
    float* intensity = new float[NUM_LIGHTS * 3];
    float* direction = new float[NUM_LIGHTS * 3];
    int* shadowmaps = new int[NUM_LIGHTS];

    int arr_index = 0;
    for (size_t i = 0; i < dir_lights_.size(); i++) {
        SceneObject* node = dir_lights_[i].first;
        glm::mat4 model_matrix = dir_lights_[i].second;
        if (!node->IsEnabled()) continue;
        // We will take the vector <0, -1, 0> and rotate it by the directional lights rotation
        glm::vec3 vals(0, -1, 0);
        glm::vec4 worldspace_direction = model_matrix * glm::vec4(vals, 0.0);
        direction[arr_index] = worldspace_direction.x;
        direction[arr_index+1] = worldspace_direction.y;
        direction[arr_index+2] = worldspace_direction.z;
        DirectionalLight* light = node->GetComponent<Light>()->as<DirectionalLight>();
        vals = light->Ambient.Get();
        ambient[arr_index] = vals.x;
        ambient[arr_index+1] = vals.y;
        ambient[arr_index+2] = vals.z;
        vals = light->GetIntensity();
        intensity[arr_index] = vals.x;
        intensity[arr_index+1] = vals.y;
        intensity[arr_index+2] = vals.z;
        arr_index += 3;
    }
    // Initialize other values to 0
    for (auto i = dir_lights_.size(); i < NUM_LIGHTS; i++) {
        direction[arr_index] = 0;
        direction[arr_index+1] = 0;
        direction[arr_index+2] = 0;
        ambient[arr_index] = 0;
        ambient[arr_index+1] = 0;
        ambient[arr_index+2] = 0;
        intensity[arr_index] = 0;
        intensity[arr_index+1] = 0;
        intensity[arr_index+2] = 0;
        arr_index += 3;
    }

    GLint loc = shader.GetUniformLocation("dir_light_intensity");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, intensity);
    loc = shader.GetUniformLocation("dir_light_ambient");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, ambient);
    loc = shader.GetUniformLocation("dir_light_direction");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, direction);

    // Point Lights
    float* position = new float[NUM_LIGHTS * 3];
    float* attenA = new float[NUM_LIGHTS];
    float* attenB = new float[NUM_LIGHTS];
    float* attenC = new float[NUM_LIGHTS];

    arr_index = 0;
    for (size_t i = 0; i < point_lights_.size(); i++) {
        SceneObject* lightnode = point_lights_[i].first;
        glm::mat4 model_matrix = point_lights_[i].second;
        if (!lightnode->IsEnabled()) continue;
        // Calculate the position of the light in eye space
        glm::vec3 vals(0, 0, 0);
        glm::vec4 worldspace_pos = model_matrix * glm::vec4(vals, 1.0);
        position[arr_index] = worldspace_pos.x;
        position[arr_index+1] = worldspace_pos.y;
        position[arr_index+2] = worldspace_pos.z;
        // Set the light properties
        PointLight* light = lightnode->GetComponent<Light>()->as<PointLight>();
        vals = light->Ambient.Get();
        ambient[arr_index] = vals.x;
        ambient[arr_index+1] = vals.y;
        ambient[arr_index+2] = vals.z;
        vals = light->GetIntensity();
        intensity[arr_index] = vals.x;
        intensity[arr_index+1] = vals.y;
        intensity[arr_index+2] = vals.z;
        attenA[i] = light->AttenA.Get();
        attenB[i] = light->AttenB.Get();
        attenC[i] = light->AttenC.Get();

        EnvironmentMap* envmap = lightnode->GetComponent<EnvironmentMap>();
        if (envmap && lightnode != ignored_node_) {
            resource_manager_.GetGLTexture(envmap->GetCubemap()).Bind(GL_TEXTURE0 + tex_counter);
            shadowmaps[i] = tex_counter;
            tex_counter++;
        } else {
            shadowmaps[i] = 0;
        }
        arr_index += 3;
    }

    // Initialize other values to 0
    for (auto i = point_lights_.size(); i < NUM_LIGHTS; i++) {
        position[arr_index] = 0;
        position[arr_index+1] = 0;
        position[arr_index+2] = 0;
        ambient[arr_index] = 0;
        ambient[arr_index+1] = 0;
        ambient[arr_index+2] = 0;
        intensity[arr_index] = 0;
        intensity[arr_index+1] = 0;
        intensity[arr_index+2] = 0;
        attenA[i] = 0;
        attenB[i] = 0;
        attenC[i] = 0;
        shadowmaps[i] = 0;
        arr_index += 3;
    }

    //for (GLuint freetex = tex_counter; freetex<last_highest_tex; freetex++) {
     //   glActiveTexture(GL_TEXTURE0+freetex);
     //   glBindTexture(GL_TEXTURE_2D, 0);
      //  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //}
    last_highest_tex = tex_counter;

    loc = shader.GetUniformLocation("point_light_intensity");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, intensity);
    loc = shader.GetUniformLocation("point_light_ambient");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, ambient);
    loc = shader.GetUniformLocation("point_light_position");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, position);
    loc = shader.GetUniformLocation("point_light_atten_const");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenC);
    loc = shader.GetUniformLocation("point_light_atten_linear");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenB);
    loc = shader.GetUniformLocation("point_light_atten_quad");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenA);
    loc = shader.GetUniformLocation("point_light_shadowmap");
    if (loc >= 0) glUniform1iv(loc, NUM_LIGHTS, shadowmaps);

    // Spot Lights
    float* cutoff = new float[NUM_LIGHTS];
    float* falloff = new float[NUM_LIGHTS];

    arr_index = 0;
    for (size_t i = 0; i < spot_lights_.size(); i++) {
        SceneObject* lightnode = spot_lights_[i].first;
        glm::mat4 model_matrix = spot_lights_[i].second;
        if (!lightnode->IsEnabled()) continue;
        // Calculate the position of the light in eye space
        glm::vec3 vals(0, 0, 0);
        glm::vec4 worldspace_pos = model_matrix * glm::vec4(vals, 1.0);
        position[arr_index] = worldspace_pos.x;
        position[arr_index+1] = worldspace_pos.y;
        position[arr_index+2] = worldspace_pos.z;
        vals = glm::vec3(0, -1, 0);
        glm::vec4 worldspace_direction = model_matrix * glm::vec4(vals, 0.0);
        direction[arr_index] = worldspace_direction.x;
        direction[arr_index+1] = worldspace_direction.y;
        direction[arr_index+2] = worldspace_direction.z;
        // Set the light properties
        SpotLight* light = lightnode->GetComponent<Light>()->as<SpotLight>();
        vals = light->Ambient.Get();
        ambient[arr_index] = vals.x;
        ambient[arr_index+1] = vals.y;
        ambient[arr_index+2] = vals.z;
        vals = light->GetIntensity();
        intensity[arr_index] = vals.x;
        intensity[arr_index+1] = vals.y;
        intensity[arr_index+2] = vals.z;
        attenA[i] = light->AttenA.Get();
        attenB[i] = light->AttenB.Get();
        attenC[i] = light->AttenC.Get();
        cutoff[i] = light->Cutoff.Get();
        falloff[i] = light->Falloff.Get();

        EnvironmentMap* envmap = lightnode->GetComponent<EnvironmentMap>();
        if (envmap && lightnode != ignored_node_) {
            resource_manager_.GetGLTexture(envmap->GetCubemap()).Bind(GL_TEXTURE0 + tex_counter);
            shadowmaps[i] = tex_counter;
            tex_counter++;
        } else {
            shadowmaps[i] = 0;
        }
        arr_index += 3;
    }

    // Initialize other values to 0
    for (auto i = spot_lights_.size(); i < NUM_LIGHTS; i++) {
        position[arr_index] = 0;
        position[arr_index+1] = 0;
        position[arr_index+2] = 0;
        direction[arr_index] = 0;
        direction[arr_index+1] = 0;
        direction[arr_index+2] = 0;
        ambient[arr_index] = 0;
        ambient[arr_index+1] = 0;
        ambient[arr_index+2] = 0;
        intensity[arr_index] = 0;
        intensity[arr_index+1] = 0;
        intensity[arr_index+2] = 0;
        attenA[i] = 0;
        attenB[i] = 0;
        attenC[i] = 0;
        cutoff[i] = 0;
        falloff[i] = 0;
        shadowmaps[i] = 0;
        arr_index += 3;
    }

//    //for (GLuint freetex = tex_counter; freetex<last_highest_tex; freetex++) {
//     //   glActiveTexture(GL_TEXTURE0+freetex);
//     //   glBindTexture(GL_TEXTURE_2D, 0);
//      //  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

//    //}
//    last_highest_tex = tex_counter;

    loc = shader.GetUniformLocation("spot_light_intensity");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, intensity);
    loc = shader.GetUniformLocation("spot_light_ambient");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, ambient);
    loc = shader.GetUniformLocation("spot_light_position");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, position);
    loc = shader.GetUniformLocation("spot_light_direction");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, direction);
    loc = shader.GetUniformLocation("spot_light_atten_const");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenC);
    loc = shader.GetUniformLocation("spot_light_atten_linear");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenB);
    loc = shader.GetUniformLocation("spot_light_atten_quad");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenA);
    loc = shader.GetUniformLocation("spot_light_cutoff");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, cutoff);
    loc = shader.GetUniformLocation("spot_light_falloff");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, falloff);
    loc = shader.GetUniformLocation("spot_light_shadowmap");
    if (loc >= 0) glUniform1iv(loc, NUM_LIGHTS, shadowmaps);

    // area light (in scene-view, renders just like point light)
    arr_index = 0;
    for (size_t i = 0; i < area_lights_.size(); i++) {
        SceneObject* lightnode = area_lights_[i].first;
        glm::mat4 model_matrix = area_lights_[i].second;
        if (!lightnode->IsEnabled()) continue;
        // Calculate the position of the light in eye space
        glm::vec3 vals(0, 0, 0);
        glm::vec4 worldspace_pos = model_matrix * glm::vec4(vals, 1.0);
        position[arr_index] = worldspace_pos.x;
        position[arr_index+1] = worldspace_pos.y;
        position[arr_index+2] = worldspace_pos.z;
        // Set the light properties
        AreaLight* light = lightnode->GetComponent<Light>()->as<AreaLight>();
        vals = light->Ambient.Get();
        ambient[arr_index] = vals.x;
        ambient[arr_index+1] = vals.y;
        ambient[arr_index+2] = vals.z;
        vals = light->GetIntensity();
        intensity[arr_index] = vals.x;
        intensity[arr_index+1] = vals.y;
        intensity[arr_index+2] = vals.z;
        attenA[i] = light->AttenA.Get();
        attenB[i] = light->AttenB.Get();
        attenC[i] = light->AttenC.Get();
        arr_index += 3;

    }

    for (auto i = area_lights_.size(); i < NUM_LIGHTS; i++) {
        position[arr_index] = 0;
        position[arr_index+1] = 0;
        position[arr_index+2] = 0;
        ambient[arr_index] = 0;
        ambient[arr_index+1] = 0;
        ambient[arr_index+2] = 0;
        intensity[arr_index] = 0;
        intensity[arr_index+1] = 0;
        intensity[arr_index+2] = 0;
        attenA[i] = 0;
        attenB[i] = 0;
        attenC[i] = 0;
        arr_index += 3;
    }

    loc = shader.GetUniformLocation("area_light_intensity");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, intensity);
    loc = shader.GetUniformLocation("area_light_ambient");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, ambient);
    loc = shader.GetUniformLocation("area_light_position");
    if (loc >= 0) glUniform3fv(loc, NUM_LIGHTS, position);
    loc = shader.GetUniformLocation("area_light_atten_const");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenC);
    loc = shader.GetUniformLocation("area_light_atten_linear");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenB);
    loc = shader.GetUniformLocation("area_light_atten_quad");
    if (loc >= 0) glUniform1fv(loc, NUM_LIGHTS, attenA);

    delete[] intensity;
    delete[] ambient;
    delete[] position;
    delete[] direction;
    delete[] attenA;
    delete[] attenB;
    delete[] attenC;
    delete[] shadowmaps;

    GLCheckError();
}