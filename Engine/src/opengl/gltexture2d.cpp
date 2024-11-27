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
#include "gltexture2d.h"
#include <opengl/glerror.h>

GLTexture2D::GLTexture2D(const Texture& tex) :
    GLTextureBase(tex, GL_TEXTURE_2D)
{
    SetTextureData(tex);
}

void GLTexture2D::SetTextureData(const Texture& tex) {
    unsigned int width = tex.GetWidth();
    unsigned int height = tex.GetHeight();
    const unsigned char* image = tex.GetImage();
    // Flip vertically due to how OpenGL has the image origin in the bottom-left corner as opposed to the top-left
    const unsigned int CHANNELS = 4; // RGBA
    unsigned int bytes_width = width * CHANNELS;
    unsigned int bytes_height = height * CHANNELS;
    unsigned char* flipped = new unsigned char[bytes_width * bytes_height];
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            unsigned int j = height - 1 - y;
            unsigned int i = x * 4;
            // Copy over a pixel
            for (unsigned int ch = 0; ch < CHANNELS; ch++)
                flipped[y * bytes_width + i + ch] = image[j * bytes_width + i + ch];
        }
    }

    // Create reference to the texture object and bind it
    Bind(0);

    // Load the data from the image buffer to define the GLTexture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, flipped);
    delete[] flipped;

    // Set texture additional properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (tex.Bilinear.Get()) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    Unbind(0);
}
