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
#include "glrenderablecubemap.h"

GLRenderableCubeMap::GLRenderableCubeMap(const RenderableCubemap &cubemap) :
    GLTextureBase(cubemap, GL_TEXTURE_CUBE_MAP)
{
    glGenRenderbuffers(1, &depth_buffer_);
    glGenFramebuffers(1, &framebuffer_);
    SetResolution(cubemap.GetResolution());
}

GLRenderableCubeMap::~GLRenderableCubeMap() {
    glDeleteFramebuffers(1, &framebuffer_);
    glDeleteRenderbuffers(1, &depth_buffer_);
}

void GLRenderableCubeMap::BindFramebuffer(int face) const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, texture_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
}

void GLRenderableCubeMap::SetResolution(int resolution) {
    resolution_ = resolution;

    // Update depth buffer dimensions
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution_, resolution);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    Bind(0);

    // Update cubemap face texture dimensions
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, resolution_, resolution_, 0, GL_RGBA, GL_FLOAT, nullptr);
    }

    // Set texture additional properties
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    Unbind(0);
}
