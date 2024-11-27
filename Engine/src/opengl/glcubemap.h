/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef GLCUBEMAP_H
#define GLCUBEMAP_H
#include "gltexturebase.h"
#include <resource/cubemap.h>

class GLCubeMap : public GLTextureBase {
public:
    GLCubeMap(const Cubemap &cubemap);
    void SetData(const Cubemap& cubemap);
};

#endif // GLCUBEMAP_H
