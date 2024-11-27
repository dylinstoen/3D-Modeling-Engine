/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef GLRESOURCEMANAGER_H
#define GLRESOURCEMANAGER_H

#include <animator.h>
#include <opengl/gltexturebase.h>
#include <opengl/glmesh.h>
#include <opengl/glshaderprogram.h>
#include <resources.h>

// TODO: Have a limited size cache depending on the GPU Memory Size
class GLResourceManager {
public:
    GLResourceManager();

    GLMesh& GetGLMesh(Mesh& mesh);
    GLShaderProgram& GetGLShaderProgram(ShaderProgram& program);
    GLTextureBase& GetGLTexture(Asset& asset);

    // TODO: Every X Render cycles we can cull the cache of unused objects
protected:
    // UID instead of name as key because name of assets might not be unique if we have two scenes loaded at once for instance.
    std::unordered_map<uint64_t, std::unique_ptr<GLMesh>> meshes_;
    std::unordered_map<uint64_t, std::unique_ptr<GLShaderProgram>> shader_programs_;
    std::unordered_map<uint64_t, std::unique_ptr<GLTextureBase>> textures_;
};

#endif // GLRESOURCEMANAGER_H
