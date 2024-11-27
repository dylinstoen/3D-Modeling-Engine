/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef OPENGLTEXTURE_H
#define OPENGLTEXTURE_H

#include "gltexturebase.h"
#include <resource/texture.h>

class GLTexture2D : public GLTextureBase {
public:
    GLTexture2D(const Texture& tex);
    void SetTextureData(const Texture& tex);
};

#endif // OPENGLTEXTURE_H
