/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef OPENGLSHADERPROGRAM_H
#define OPENGLSHADERPROGRAM_H

#include <glinclude.h>
#include <resource/shaderprogram.h>

class GLSLShader {
public:
    GLSLShader(const std::string& source, GLenum GL_shader_type);
    ~GLSLShader();
    GLint GetID();
private:
    GLint shader_;
};

class GLShaderProgram : public ShaderProgram {
public:
    // List of Vertex Attribute Locations
    static const std::map<std::string, GLint> AttributeLocations() {
        static const std::map<std::string, GLint> attribute_locations = {
            {"position", 0},
            {"normal", 1},
            {"color", 2},
            {"texcoord", 3},
            {"binormal", 4},
            {"tangent", 5}
        };
        return attribute_locations;
    }

    GLShaderProgram(const std::string& name);
    GLShaderProgram(const ShaderProgram& program);

    GLuint GetProgram() { return program_; }
    virtual bool IsValidShaderProgram() const override { return ValidateShaderProgram(program_); }
    virtual void SetShader(const std::string& name, const std::string& source, ShaderType shader_type) override;
    virtual std::vector<std::pair<std::string, DataType>> GetShaderInputs() const override;
    std::map<std::string, std::pair<GLint, DataType>> GetUniformLocations() const;
    GLint GetUniformLocation(const std::string& name);
protected:
    const std::string vert_source_ =
        "#version 150\n"
        "in vec3 position;"
        "uniform mat4 model_matrix;"
        "uniform mat4 view_matrix;"
        "uniform mat4 projection_matrix;"
        "void main() {"
        "   gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);"
        "}";

    const std::string frag_source_ =
        "#version 150\n"
        "out vec4 outColor;"
        "void main() {"
        "   outColor = vec4(1, 0.28, 0.95, 1);"
        "}";

    // Queries the active uniforms in the shader program via introspection
    void QueryUniforms(GLuint shader_program);
    bool ValidateShaderProgram(GLuint shader_program) const;
    GLuint program_;
    std::map<std::string, std::pair<GLint, DataType>> uniform_locations_;
    std::vector<std::pair<std::string, DataType>> uniforms_list_;
    std::map<GLenum, std::unique_ptr<GLSLShader>> attached_shaders_;
    // Internal calls that actually do the work
    void OnSetVertexShader(std::string path);
    void OnSetFragmentShader(std::string path);
    void OnSetGeometryShader(std::string path);
    void OnSetTraceCompatible(bool c);
    void OnSetShader(const std::string& path, GLenum GL_shader_type);
    void OnSetShader(const std::string& name, const std::string& source, GLenum GL_shader_type);
};

#endif // OPENGLSHADERPROGRAM_H
