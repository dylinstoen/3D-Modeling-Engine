/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef GLTEXTUREBASE_H
#define GLTEXTUREBASE_H

#include <glinclude.h>
#include <resource/cacheable.h>

class GLTextureBase : public Cacheable {
public:
    GLTextureBase(const Cacheable& cacheable, GLuint texture_type = GL_TEXTURE_2D);
    virtual ~GLTextureBase();

    GLuint GetTextureId() const { return texture_; }
    void Bind(GLenum texture_unit) const;
    void Unbind(GLenum texture_unit) const;
protected:
    GLuint texture_;
    GLuint texture_type_;
};

#endif // GLTEXTUREBASE_H
