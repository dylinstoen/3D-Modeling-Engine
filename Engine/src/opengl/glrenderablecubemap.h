/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef GLRENDERABLECUBEMAP_H
#define GLRENDERABLECUBEMAP_H

#include "gltexturebase.h"
#include <resource/cubemap.h>

class GLRenderableCubeMap : public GLTextureBase {
public:
    GLRenderableCubeMap(const RenderableCubemap &cubemap);
    ~GLRenderableCubeMap();

    void BindFramebuffer(int face) const;
    void SetResolution(int resolution);

protected:
    GLuint framebuffer_;
    GLuint depth_buffer_;
    int resolution_;
};

#endif // GLRENDERABLECUBEMAP_H
