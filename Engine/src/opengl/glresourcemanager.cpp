/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "glresourcemanager.h"
#include <opengl/glcubemap.h>
#include <opengl/glrenderablecubemap.h>
#include <opengl/gltexture2d.h>
#include <opengl/glrenderabletexture.h>

GLResourceManager::GLResourceManager() {

}

GLMesh& GLResourceManager::GetGLMesh(Mesh &mesh) {
    uint64_t uid = mesh.GetUID();
    if (meshes_.count(uid) < 1) {
        meshes_[uid] = std::make_unique<GLMesh>(mesh);
        meshes_[uid]->MarkUpdated();
    } else if (meshes_[uid]->IsDirty()) {
        meshes_[uid]->SetMeshData(mesh);
        meshes_[uid]->MarkUpdated();
    }
    return *meshes_[uid];
}

GLTextureBase& GLResourceManager::GetGLTexture(Asset &asset) {
    uint64_t uid = asset.GetUID();
    // TODO: Check if the texture is dirty (not possible atm)
    if (textures_.count(uid) < 1) {
        switch (asset.GetType()) {
            case AssetType::Texture:
                textures_[uid] = std::make_unique<GLTexture2D>(dynamic_cast<Texture&>(asset));
                break;
            case AssetType::RenderableTexture:
                textures_[uid] = std::make_unique<GLRenderableTexture>(dynamic_cast<RenderableTexture&>(asset));
                break;
            case AssetType::Cubemap:
                textures_[uid] = std::make_unique<GLCubeMap>(dynamic_cast<Cubemap&>(asset));
                break;
            case AssetType::RenderableCubemap:
                textures_[uid] = std::make_unique<GLRenderableCubeMap>(dynamic_cast<RenderableCubemap&>(asset));
                break;
            default:
                // Other types don't have direct opengl representations
                break;
        }
        textures_[uid]->MarkUpdated();
    } else if (textures_[uid]->IsDirty()) {
        switch (asset.GetType()) {
            case AssetType::Texture: {
                GLTexture2D* gltex = dynamic_cast<GLTexture2D*>(textures_[uid].get());
                gltex->SetTextureData(dynamic_cast<Texture&>(asset));
                break; }
            case AssetType::RenderableTexture: {
                GLRenderableTexture* gltex = dynamic_cast<GLRenderableTexture*>(textures_[uid].get());
                gltex->SetResolution(dynamic_cast<RenderableTexture&>(asset).GetResolution());
                break; }
            case AssetType::Cubemap: {
                GLCubeMap* glcubemap = dynamic_cast<GLCubeMap*>(textures_[uid].get());
                glcubemap->SetData(dynamic_cast<Cubemap&>(asset));
                break; }
            case AssetType::RenderableCubemap: {
                GLRenderableCubeMap* glcubemap = dynamic_cast<GLRenderableCubeMap*>(textures_[uid].get());
                glcubemap->SetResolution(dynamic_cast<RenderableCubemap&>(asset).GetResolution());
                break; }
            default:
                // Other types don't have direct opengl representations
                break;
        }
        textures_[uid]->MarkUpdated();
    }
    return *textures_[uid];
}

GLShaderProgram& GLResourceManager::GetGLShaderProgram(ShaderProgram& program) {
    uint64_t uid = program.GetUID();
    // TODO: Check if the texture is dirty (not possible atm)
    if (shader_programs_.count(uid) < 1) {
        shader_programs_[uid] = std::make_unique<GLShaderProgram>(program);
        shader_programs_[uid]->MarkUpdated();
    } else if (shader_programs_[uid]->IsDirty()) {
        if (!program.GetShaderText(ShaderType::Vertex).empty())
            shader_programs_[uid]->SetShader("Vertex", program.GetShaderText(ShaderType::Vertex), ShaderType::Vertex);
        if (!program.GetShaderText(ShaderType::Fragment).empty())
            shader_programs_[uid]->SetShader("Fragment", program.GetShaderText(ShaderType::Fragment), ShaderType::Fragment);
        if (!program.GetShaderText(ShaderType::Geometry).empty())
            shader_programs_[uid]->SetShader("Geometry", program.GetShaderText(ShaderType::Geometry), ShaderType::Geometry);
        shader_programs_[uid]->MarkUpdated();
    }
    return *shader_programs_[uid];
}
