/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "glextinclude.h"
#include "gltexturebase.h"

GLTextureBase::GLTextureBase(const Cacheable& cacheable, GLuint texture_type)
    : Cacheable(&cacheable),
      texture_type_(texture_type)
{
    glGenTextures(1, &texture_);
}
GLTextureBase::~GLTextureBase()
{
    glDeleteTextures(1, &texture_);
}

void GLTextureBase::Bind(GLenum texture_unit) const {
    if (texture_unit == 0) texture_unit = GL_TEXTURE0;
    glActiveTexture(texture_unit);
    glBindTexture(texture_type_, texture_);
}

void GLTextureBase::Unbind(GLenum texture_unit) const {
    if (texture_unit == 0) texture_unit = GL_TEXTURE0;
    glActiveTexture(texture_unit);
    glBindTexture(texture_type_, 0);
}

