/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <fileio.h>
#include <meshprocessing.h>
#include <glm/gtx/transform.hpp>
#include <resource/assetmanager.h>
#include <resource/importers.h>
#include <resource/cubemap.h>
#include <resource/texture.h>
#include <resource/material.h>
#include <resource/mesh.h>
#include <resource/shaderprogram.h>
#include <resource/shaderfactory.h>
#include <resource/shapes.h>
#include <scene/components/transform.h>

template<> AssetManager* Singleton<AssetManager>::_instance_ = nullptr;

AssetManager::AssetManager(ShaderFactory& shader_factory) :
    Singleton<AssetManager>(),
    shader_factory_(&shader_factory)
{
    // Setup default assets
    static const unsigned char texture_data[16] = {
        255, 0, 255, 255, 0, 0, 0, 255,
        0, 0, 0, 255, 255, 0, 255, 255
    };
    static const unsigned char* cubemap_data[] = {
        texture_data,
        texture_data,
        texture_data,
        texture_data,
        texture_data,
        texture_data
    };
    default_texture_ = std::make_unique<Texture>("Default Texture", 2, 2, texture_data);
    default_shader_ = shader_factory_->CreateShaderProgram("Default Shader");
    default_material_ = std::make_unique<Material>("Default Material", default_shader_.get());
    default_cubemap_ = std::make_unique<Cubemap>("Default Cubemap", 2, cubemap_data);

    // Create the Cube mesh
    auto cube_mesh = CreateMesh("Cube");
    cube_mesh->SetHidden(false);
    cube_mesh->SetPositions(std::vector<float>(Cube::Vertices()));
    cube_mesh->SetNormals(std::vector<float>(Cube::Normals()));
    cube_mesh->SetColors(std::vector<float>(Cube::Colors()));
    cube_mesh->SetUVs(std::vector<float>(Cube::UVs()));
    cube_mesh->SetTriangles(std::vector<unsigned int>(Cube::Triangles()));

    auto cube2_mesh = CreateMesh("Hollow Cube");
    cube2_mesh->SetHidden(false);
    MeshProcessing::FlipNormals(*cube_mesh, *cube2_mesh);
    cube2_mesh->Append(*cube_mesh,glm::scale(glm::vec3(1.1,1.1,1.1)));

    // Create the Pyramid mesh
    auto pyramid_mesh = CreateMesh("Pyramid");
    pyramid_mesh->SetHidden(false);
    pyramid_mesh->SetPositions(std::vector<float>(Pyramid::Vertices()));
    pyramid_mesh->SetNormals(std::vector<float>(Pyramid::Normals()));
    pyramid_mesh->SetColors(std::vector<float>(Pyramid::Colors()));
    pyramid_mesh->SetUVs(std::vector<float>(Pyramid::UVs()));
    pyramid_mesh->SetTriangles(std::vector<unsigned int>(Pyramid::Triangles()));

    // Create the Banana Mesh
    auto banana_mesh = CreateMesh("Banana");
    banana_mesh->SetHidden(false);
    banana_mesh->SetPositions(std::vector<float>(Banana::Vertices()));
    banana_mesh->SetNormals(std::vector<float>(Banana::Normals()));
    banana_mesh->SetTriangles(std::vector<unsigned int>(Banana::Triangles()));

    // Create the Arrow mesh
    auto arrow_mesh = CreateMesh("Arrow");
    arrow_mesh->SetHidden(false);
    Transform shaft_transform, arrowhead_transform;
    shaft_transform.Scale.Set(glm::vec3(0.01f, 0.28f, 0.01f));
    shaft_transform.Translation.Set(glm::vec3(0.0f, 0.14f, 0.0f));
    arrow_mesh->Append(*cube_mesh, shaft_transform.GetMatrix());
    arrowhead_transform.Scale.Set(glm::vec3(0.07f, 0.07f, 0.07f));
    arrowhead_transform.Translation.Set(glm::vec3(0.0f, 0.0f, 0.0f));
    arrowhead_transform.Rotation.Set(glm::vec3(180.f, 0.f, 0.f));
    arrow_mesh->Append(*pyramid_mesh, arrowhead_transform.GetMatrix());

    // Import standard meshes
    LoadMesh("Teapot", "assets/teapot.obj", true);
    LoadMesh("Spikey", "assets/spikey.obj", true);
    LoadMesh("Bunny", "assets/bunny.obj", true);
    // These take a while to load
    //LoadMesh("Dragon", "assets/dragon.ply", true);
    //LoadMesh("Buddha", "assets/buddha.ply", true);

    // ------- Basic Materials -------
    // Flat textured shader and material
    ShaderProgram* tex_shader = CreateShaderProgram("Textured Shader", false);
    tex_shader->VertexShader.Set("assets/texture.vert");
    tex_shader->FragmentShader.Set("assets/texture.frag");
    Material* textured_material = CreateMaterial("Textured Material", false);
    textured_material->Shader.Set(tex_shader);
    LoadTexture("Checkers Texture", "assets/checkers.png");
    auto texture = GetTexture("Checkers Texture");
    textured_material->Uniforms.Get<TextureProperty>("DiffuseMap")->Set(texture);

    // Emissive shader and material
    ShaderProgram* emissive_shader = CreateShaderProgram("Emissive Shader", false);
    emissive_shader->SetShader("Blinn Phong Vert", blinn_phong_vert_src_, ShaderType::Vertex);
    emissive_shader->FragmentShader.Set("assets/emissive.frag");
    Material* emissive_material = CreateMaterial("Emissive", false);
    emissive_material->Shader.Set(emissive_shader);
    emissive_material->Uniforms.Get<DoubleProperty>("ConstantAttenuation")->Set(1.f);

    // Blinn Phong Shader and material
    ShaderProgram* blinn_phong_shader = CreateShaderProgram("Blinn-Phong Shader", false);
    blinn_phong_shader->SetShader("Blinn Phong Vert", blinn_phong_vert_src_, ShaderType::Vertex);
    blinn_phong_shader->SetShader("Blinn Phong Frag", blinn_phong_frag_src_, ShaderType::Fragment);
    blinn_phong_shader->TraceCompatible.Set(true);
//    blinn_phong_shader->SetShader("assets/blinn-phong.vert", ShaderType::Vertex);
//    blinn_phong_shader->SetShader("assets/blinn-phong.frag", ShaderType::Fragment);
    Material* blinn_phong_mat = CreateMaterial("Blinn-Phong Material", false);
    blinn_phong_mat->Shader.Set(blinn_phong_shader);
    blinn_phong_mat->Uniforms.Get<DoubleProperty>("Shininess")->Set(20.0f);
    blinn_phong_mat->Uniforms.Get<TextureProperty>("Diffuse")->SetColor(glm::vec3(0.75f, 0.0f, 0.0f));

    // Blinn Phong Shader (Spotlight) and material
    ShaderProgram* blinn_phong_spotlight_shader = CreateShaderProgram("Blinn-Phong Spotlight Shader", false);
    blinn_phong_spotlight_shader->SetShader("Blinn Phong Vert", blinn_phong_vert_src_, ShaderType::Vertex);
    blinn_phong_spotlight_shader->SetShader("Blinn Phong Frag", blinn_phong_frag_src_, ShaderType::Fragment);
    blinn_phong_spotlight_shader->TraceCompatible.Set(true);
    Material* blinn_phong_spotlight_mat = CreateMaterial("Blinn-Phong Spotlight Material", false);
    blinn_phong_spotlight_mat->Shader.Set(blinn_phong_spotlight_shader);
    blinn_phong_spotlight_mat->Uniforms.Get<DoubleProperty>("Shininess")->Set(20.0f);
    blinn_phong_spotlight_mat->Uniforms.Get<TextureProperty>("Diffuse")->SetColor(glm::vec3(0.75f, 0.0f, 0.0f));

    // Alpha shader and material
    ShaderProgram* alpha_shader = CreateShaderProgram("Alpha Test Shader", false);
    alpha_shader->VertexShader.Set("assets/texture.vert");
    alpha_shader->SetShader("Alpha Frag", alpha_frag_src_, ShaderType::Fragment);
    Material* alpha_material = CreateMaterial("Alpha Test Material", false);
    alpha_material->Shader.Set(alpha_shader);
    LoadTexture("Alpha Test Texture", "assets/alpha_test.png");
    auto alpha_texture = GetTexture("Alpha Test Texture");
    alpha_material->Uniforms.Get<TextureProperty>("DiffuseMap")->Set(alpha_texture);

    // Toon Shader and material
    ShaderProgram* toon_shader = CreateShaderProgram("Toon Shader", false);
    toon_shader->SetShader("Toon Vert", blinn_phong_vert_src_, ShaderType::Vertex);
    toon_shader->SetShader("Toon Frag", toon_frag_src_, ShaderType::Fragment);
    toon_shader->TraceCompatible.Set(true);
    Material* toon_mat = CreateMaterial("Toon Material", false);
    toon_mat->Shader.Set(toon_shader);
    toon_mat->Uniforms.Get<DoubleProperty>("Shininess")->Set(20.0f);
    toon_mat->Uniforms.Get<TextureProperty>("Diffuse")->SetColor(glm::vec3(0.75f, 0.0f, 0.0f));

    // Depth Map Shader and material
    ShaderProgram* depth_shader = CreateShaderProgram("Depth Shader", false);
    depth_shader->VertexShader.Set("assets/internal/position.vert");
    depth_shader->SetShader("Depth Frag", depth_frag_src, ShaderType::Fragment);
    Material* depth_mat = CreateMaterial("Depth Map Material", false);
    depth_mat->Shader.Set(depth_shader);

    // ------- Internal Materials -------
    // Unlit shader and material
    ShaderProgram* unlit_shader = CreateShaderProgram("_internal Unlit Shader");
    unlit_shader->VertexShader.Set("assets/default.vert");
    unlit_shader->FragmentShader.Set("assets/default.frag");

    Material* unlit_mat = CreateMaterial("_internal Unlit Material");
    unlit_mat->Shader.Set(unlit_shader);
    unlit_mat->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.5f, 0.5f, 0.5f));

    // Colored materials
    Material* red_material = CreateMaterial("_internal Unlit Red");
    red_material->Shader.Set(unlit_shader);
    red_material->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.84f, 0.f, 0.f));

    Material* green_material = CreateMaterial("_internal Unlit Green");
    green_material->Shader.Set(unlit_shader);
    green_material->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.13f, 0.7f, 0.3f));

    Material* blue_material = CreateMaterial("_internal Unlit Blue");
    blue_material->Shader.Set(unlit_shader);
    blue_material->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.25f, 0.28f, 0.8f));

    Material* yellow_material = CreateMaterial("_internal Unlit Yellow");
    yellow_material->Shader.Set(unlit_shader);
    yellow_material->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.7f, 0.7f, 0.f));

    Material* gray_material = CreateMaterial("_internal Unlit Gray");
    gray_material->Shader.Set(unlit_shader);
    gray_material->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(0.6f, 0.6f, 0.6f));

    // ------- UI Shaders -------
    // Normal shader and material
    ShaderProgram* normals_shader = CreateShaderProgram("_internal Normals Shader");
    normals_shader->VertexShader.Set("assets/normals.vert");
    normals_shader->GeometryShader.Set("assets/normals.geom");
    normals_shader->FragmentShader.Set("assets/normals.frag");
    Material* normals_mat = CreateMaterial("_internal Normals Material");
    normals_mat->Shader.Set(normals_shader);
    normals_mat->Uniforms.Get<DoubleProperty>("NormalLength")->Set(0.1f);
    normals_mat->Uniforms.Get<ColorProperty>("NormalColor")->Set(glm::vec3(0.0f, 1.0f, 0.0f));

    // Wireframe shader and material
    ShaderProgram* wireframe_shader = CreateShaderProgram("_internal Wireframe Shader");
    wireframe_shader->VertexShader.Set("assets/wireframe.vert");
    wireframe_shader->GeometryShader.Set("assets/wireframe.geom");
    wireframe_shader->FragmentShader.Set("assets/wireframe.frag");
    Material* wireframe_mat = CreateMaterial("_internal Wireframe Material");
    wireframe_mat->Shader.Set(wireframe_shader);

    // ------- Mouse Interaction Shaders -------
    // Color picking shader and material
    ShaderProgram* picking_shader = CreateShaderProgram("_mouse Color Picking Shader");
    picking_shader->VertexShader.Set("assets/internal/position.vert");
    picking_shader->FragmentShader.Set("assets/internal/position.frag");
    Material* picking_mat = CreateMaterial("_mouse Color Picking Material");
    picking_mat->Shader.Set(picking_shader);

    Material* highlight_mat = CreateMaterial("_mouse Highlight Material");
    highlight_mat->Shader.Set(unlit_shader);
    highlight_mat->Uniforms.Get<ColorProperty>("Color")->Set(glm::vec3(1.f,1.f,0.f));
}

AssetManager::~AssetManager() {
    AssetCreated.Clear();
    AssetDeleted.Clear();
}

void AssetManager::LoadTexture(const std::string& name, const std::string& path) {
    try {
        if (textures_.count(name) > 0) UnloadTexture(name);
        textures_.emplace(std::make_pair(name, Importers::ImportTexture(name, path)));
        AssetCreated.Emit(*textures_[name]);
    } catch (const FileIOException& e) {
        Debug::Log.WriteLine(e.what(), Priority::Error);
    }
}

void AssetManager::LoadCubemap(const std::string& name, const std::string& path) {
    try {
        if (cubemaps_.count(name) > 0) UnloadCubemap(name);
        cubemaps_.emplace(std::make_pair(name, Importers::ImportCubemap(name, path)));
        AssetCreated.Emit(*cubemaps_[name]);
    } catch (const FileIOException& e) {
        Debug::Log.WriteLine(e.what(), Priority::Error);
    }
}

void AssetManager::LoadMesh(const std::string& name, const std::string& path, bool internal) {
    try {
        if (meshes_.count(name) > 0) UnloadMesh(name);
        meshes_.emplace(std::make_pair(name, Importers::ImportMesh(name, path)));
        if (internal) {
            meshes_[name]->MakeInternal();
        }
        AssetCreated.Emit(*meshes_[name]);
    } catch (const FileIOException& e) {
        Debug::Log.WriteLine(e.what(), Priority::Error);
    }
}

Mesh* AssetManager::CreateMesh(const std::string &name, MeshType meshtype, bool internal, bool hidden) {
    if (meshes_.count(name) > 0) return nullptr;
    meshes_[name] = std::make_unique<Mesh>(name, meshtype);
    if (internal) meshes_[name]->MakeInternal();
    if (hidden) meshes_[name]->SetHidden();

    AssetCreated.Emit(*meshes_[name]);
    return meshes_[name].get();
}

Material* AssetManager::CreateMaterial(const std::string &name, bool internal) {
    if (materials_.count(name) > 0) return nullptr;
    auto default_shader = default_shader_.get();
    materials_[name] = std::make_unique<Material>(name, default_shader);
    if (internal) {
        materials_[name]->MakeInternal();
        materials_[name]->SetHidden();
    }
    AssetCreated.Emit(*materials_[name]);
    return materials_[name].get();
}

ShaderProgram* AssetManager::CreateShaderProgram(const std::string &name, bool internal) {
    if (shader_programs_.count(name) > 0) return nullptr;
    shader_programs_[name] = shader_factory_->CreateShaderProgram(name);
    if (internal) {
        shader_programs_[name]->MakeInternal();
        shader_programs_[name]->SetHidden();
    }
    AssetCreated.Emit(*shader_programs_[name]);
    return shader_programs_[name].get();
}

void AssetManager::UnloadTexture(const std::string& name) {
    if (textures_.count(name) < 1) return;
    AssetDeleted.Emit(textures_[name]->GetUID());
    textures_[name]->Deleted.Emit();
    textures_.erase(name);
}

void AssetManager::UnloadCubemap(const std::string& name) {
    if (cubemaps_.count(name) < 1) return;
    AssetDeleted.Emit(cubemaps_[name]->GetUID());
    cubemaps_[name]->Deleted.Emit();
    cubemaps_.erase(name);
}

void AssetManager::UnloadMaterial(const std::string& name) {
    if (materials_.count(name) < 1) return;
    AssetDeleted.Emit(materials_[name]->GetUID());
    materials_[name]->Deleted.Emit();
    materials_.erase(name);
}

void AssetManager::UnloadMesh(const std::string& name) {
    if (meshes_.count(name) < 1) return;
    AssetDeleted.Emit(meshes_[name]->GetUID());
    meshes_[name]->Deleted.Emit();
    meshes_.erase(name);
}

void AssetManager::UnloadShaderProgram(const std::string& name) {
    if (shader_programs_.count(name) < 1) return;
    AssetDeleted.Emit(shader_programs_[name]->GetUID());
    shader_programs_[name]->Deleted.Emit();
    shader_programs_.erase(name);
}

Texture* AssetManager::GetTexture(const std::string& name) {
    if (name == "Default Texture") return default_texture_.get();
    else if (textures_.count(name) < 1) {
        Debug::Log.WriteLine("Texture not imported \"" + name + "\"", Priority::Error);
        return default_texture_.get();
    }

    return textures_[name].get();
}

Cubemap* AssetManager::GetCubemap(const std::string& name) {
    if (name == "Default Cubemap") return default_cubemap_.get();
    else if (cubemaps_.count(name) < 1) {
        Debug::Log.WriteLine("Cubemap not imported \"" + name + "\"", Priority::Error);
        return default_cubemap_.get();
    }

    return cubemaps_[name].get();
}

Material* AssetManager::GetMaterial(const std::string& name) {
    if (name == "Default Material") return default_material_.get();
    else if (materials_.count(name) < 1) {
        Debug::Log.WriteLine("Material not imported \"" + name + "\"", Priority::Error);
        return default_material_.get();
    }

    return materials_[name].get();
}

Mesh* AssetManager::GetMesh(const std::string& name) {
    if (meshes_.count(name) < 1) {
        // Debug::Log.WriteLine("Mesh not imported \"" + name + "\"", Priority::Error);
        return meshes_["default"].get();
    }

    return meshes_[name].get();
}

ShaderProgram* AssetManager::GetShaderProgram(const std::string& name) {
    if (name == "Default Shader") return default_shader_.get();
    else if (shader_programs_.count(name) < 1) {
        Debug::Log.WriteLine("ShaderProgram not imported \"" + name + "\"", Priority::Error);
        return default_shader_.get();
    }

    return shader_programs_[name].get();
}

Texture *AssetManager::GetOrCreateSolidTexture(glm::vec3 color)
{
    unsigned char r = color.x * 255;
    unsigned char g = color.y * 255;
    unsigned char b = color.z * 255;

    unsigned int i = (unsigned int)r | ((unsigned int)g<<8) | ((unsigned int)b<<16);

    if (solid_textures_.find(i) == solid_textures_.end()) {

        unsigned char tinytexture[16] = {
            r,g,b,255,r,g,b,255,r,g,b,255,r,g,b,255
        };

        solid_textures_.emplace(i, std::make_unique<Texture>(std::string("col")+std::to_string(i), 2,2, tinytexture));
    }

    return solid_textures_[i].get();
}

std::vector<Texture *> AssetManager::GetTextures() const {
    std::vector<Texture*> ret;
    for (auto& kv : textures_) ret.push_back(kv.second.get());
    return ret;
}

std::vector<Cubemap *> AssetManager::GetCubemaps() const {
    std::vector<Cubemap*> ret;
    for (auto& kv : cubemaps_) ret.push_back(kv.second.get());
    return ret;
}

std::vector<Material *> AssetManager::GetMaterials() const {
    std::vector<Material*> ret;
    for (auto& kv : materials_) ret.push_back(kv.second.get());
    return ret;
}

std::vector<Mesh *> AssetManager::GetMeshes() const {
    std::vector<Mesh*> ret;
    for (auto& kv : meshes_) ret.push_back(kv.second.get());
    return ret;
}

std::vector<ShaderProgram *> AssetManager::GetShaderPrograms() const {
    std::vector<ShaderProgram*> ret;
    for (auto& kv : shader_programs_) ret.push_back(kv.second.get());
    return ret;
}

void AssetManager::Refresh() {
    // Reload shaders
    for (auto & kv : shader_programs_) {
        ShaderProgram* prog = kv.second.get();
        if (prog->VertexShader.IsSet()) prog->VertexShader.Set(prog->VertexShader.Get());
        if (prog->FragmentShader.IsSet()) prog->FragmentShader.Set(prog->FragmentShader.Get());
        if (prog->GeometryShader.IsSet()) prog->GeometryShader.Set(prog->GeometryShader.Get());
    }
}
