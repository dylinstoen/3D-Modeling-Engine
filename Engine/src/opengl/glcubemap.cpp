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
#include "glcubemap.h"

GLCubeMap::GLCubeMap(const Cubemap &cubemap) :
    GLTextureBase(cubemap, GL_TEXTURE_CUBE_MAP)
{
    SetData(cubemap);
}

void GLCubeMap::SetData(const Cubemap &cubemap) {
    unsigned int resolution = cubemap.GetResolution();

    // Create reference to the texture object and bind it
    Bind(0);

    for (int face = 0; face < Cubemap::NUM_CUBEMAP_FACES; face++) {
        const unsigned char* image = cubemap.GetFace(face);
        // Flip vertically due to how OpenGL has the image origin in the bottom-left corner as opposed to the top-left
        // Load the data from the image buffer to define the GLTexture
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face, 0, GL_RGBA, resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }

    // Set texture additional properties
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    Unbind(0);
}
