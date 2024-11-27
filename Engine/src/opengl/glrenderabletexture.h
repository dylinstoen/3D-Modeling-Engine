/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef GLRENDERABLETEXTURE_H
#define GLRENDERABLETEXTURE_H

#include "gltexturebase.h"
#include <resource/texture.h>

class GLRenderableTexture : public GLTextureBase {
public:
    GLRenderableTexture(const RenderableTexture &texture);
    ~GLRenderableTexture();

    void BindFramebuffer() const;
    void SetResolution(int width, int height);
    void SetResolution(glm::ivec2 resolution) { SetResolution(resolution.x, resolution.y); }

    glm::ivec2 GetResolution() const { return glm::ivec2(width_, height_); }

protected:
    GLuint framebuffer_;
    GLuint depth_buffer_;
    int width_, height_;
};

#endif // GLRENDERABLETEXTURE_H
