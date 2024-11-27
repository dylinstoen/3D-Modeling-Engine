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
#include "glrenderabletexture.h"

GLRenderableTexture::GLRenderableTexture(const RenderableTexture &texture) :
    GLTextureBase(texture, GL_TEXTURE_2D)
{
    glGenRenderbuffers(1, &depth_buffer_);
    glGenFramebuffers(1, &framebuffer_);
    SetResolution(texture.GetWidth(), texture.GetHeight());
}

GLRenderableTexture::~GLRenderableTexture() {
    glDeleteFramebuffers(1, &framebuffer_);
    glDeleteRenderbuffers(1, &depth_buffer_);
}

void GLRenderableTexture::BindFramebuffer() const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
}

void GLRenderableTexture::SetResolution(int width, int height) {
    width_ = width;
    height_ = height;

    // Update depth buffer dimensions
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width_, height_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    Bind(0);

    // Update texture size
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_FLOAT, nullptr);

    // Set texture additional properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    Unbind(0);
}
