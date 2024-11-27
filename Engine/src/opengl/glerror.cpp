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
#include "glerror.h"
#include <glinclude.h>

bool _GLCheckError(const char *file, int line) {
    GLenum err; bool found_error = false;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::string error;
        std::stringstream error_stream;
        found_error = true;

        switch(err) {
                case GL_INVALID_OPERATION:
                    error="INVALID_OPERATION";
                    break;
                case GL_INVALID_ENUM:
                    error="INVALID_ENUM";
                    break;
                case GL_INVALID_VALUE:
                    error="INVALID_VALUE";
                    break;
                case GL_OUT_OF_MEMORY:
                    error="OUT_OF_MEMORY";
                    break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    error="INVALID_FRAMEBUFFER_OPERATION";
                    break;
        }

        error_stream << "GL_" << error << " - "<< file << ":" << line;
        Debug::Log.WriteLine(error_stream.str(), Priority::Error);
    }
    return found_error;
}
