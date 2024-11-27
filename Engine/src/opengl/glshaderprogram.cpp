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
#include "glshaderprogram.h"
#include <assert.h>
#include <fileio.h>
#include <algorithm>
#include <opengl/glerror.h>

GLSLShader::GLSLShader(const std::string &source, GLenum GL_shader_type) {
    // Ask OpenGL to allocate a Shader object
    shader_ = glCreateShader(GL_shader_type);
    // Upload the source code into the Shader object
    const char* source_cstr = source.c_str();
    glShaderSource(shader_, 1, &source_cstr, NULL); // If last param is NULL, source is assumed to be null-terminated
}

GLSLShader::~GLSLShader() { glDeleteShader(shader_); }

GLint GLSLShader::GetID() { return shader_; }

GLShaderProgram::GLShaderProgram(const std::string& name) :
    ShaderProgram(name)
{
    program_ = glCreateProgram();

    VertexShader.ValueSet.Connect(this, &GLShaderProgram::OnSetVertexShader);
    FragmentShader.ValueSet.Connect(this, &GLShaderProgram::OnSetFragmentShader);
    GeometryShader.ValueSet.Connect(this, &GLShaderProgram::OnSetGeometryShader);
    TraceCompatible.ValueSet.Connect(this, &GLShaderProgram::OnSetTraceCompatible);
}

GLShaderProgram::GLShaderProgram(const ShaderProgram& program) :
    ShaderProgram(program.GetName(), &program)
{
    program_ = glCreateProgram();
    VertexShader.ValueSet.Connect(this, &GLShaderProgram::OnSetVertexShader);
    FragmentShader.ValueSet.Connect(this, &GLShaderProgram::OnSetFragmentShader);
    GeometryShader.ValueSet.Connect(this, &GLShaderProgram::OnSetGeometryShader);
    TraceCompatible.ValueSet.Connect(this, &GLShaderProgram::OnSetTraceCompatible);

    VertexShader.Set(program.VertexShader.Get());
    GeometryShader.Set(program.GeometryShader.Get());
    FragmentShader.Set(program.FragmentShader.Get());
    TraceCompatible.Set(program.TraceCompatible.Get());

    if (!program.GetShaderText(ShaderType::Vertex).empty())
        SetShader("Vertex", program.GetShaderText(ShaderType::Vertex), ShaderType::Vertex);
    if (!program.GetShaderText(ShaderType::Geometry).empty())
        SetShader("Geometry", program.GetShaderText(ShaderType::Geometry), ShaderType::Geometry);
    if (!program.GetShaderText(ShaderType::Fragment).empty())
        SetShader("Fragment", program.GetShaderText(ShaderType::Fragment), ShaderType::Fragment);

}

bool GLShaderProgram::ValidateShaderProgram(GLuint shader_program) const {
    glValidateProgram(shader_program);
    GLint validate_status;
    glGetProgramiv(shader_program, GL_VALIDATE_STATUS, &validate_status);   

    return validate_status == GL_TRUE;
}

// TODO: Destructor?

void CompileShader(GLuint shader) {
    // Compile the shader
    glCompileShader(shader);

    // Check if the shader compiled successfully
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(success == GL_FALSE) {
        // Retrieves the compile log into a buffer
        GLint logSize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        char* buffer = new char[logSize];
        glGetShaderInfoLog(shader, logSize, NULL, buffer);
        // Throw a ShaderException with the log
        std::string log(buffer);
        delete [] buffer;
        throw ShaderCompileException(log);
    }
}

void LinkProgram(GLuint program) {
    // Compile the shader
    glLinkProgram(program);

    // Check if the shader compiled successfully
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(success == GL_FALSE) {
        // Retrieves the compile log into a buffer
        GLint logSize;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        char* buffer = new char[logSize];
        glGetProgramInfoLog(program, logSize, NULL, buffer);
        // Throw a ShaderException with the log
        std::string log(buffer);
        delete [] buffer;
        throw ProgramLinkException(log);
    }
}

bool IsColorType(const std::string& name) {
    int len = name.size();
    if (len >= 5) {
        std::string last_5 = name.substr(len - 5, 5);
        std::transform(last_5.begin(), last_5.end(), last_5.begin(), ::tolower);
        return last_5 == "color";
    } else {
        return false;
    }
}

DataType ConvertType(const std::string& name, GLenum gl_type) {
    switch(gl_type) {
        case GL_FLOAT:
            return DataType::Float;
        case GL_FLOAT_VEC3:
            if (IsColorType(name)) return DataType::ColorRGB;
            else return DataType::Float3;
        case GL_FLOAT_VEC4:
            if (IsColorType(name)) return DataType::ColorRGBA;
            else return DataType::Float4;
        case GL_DOUBLE:
            return DataType::Double;
        case GL_DOUBLE_VEC3:
            return DataType::Double3;
        case GL_DOUBLE_VEC4:
            return DataType::Double4;
        case GL_INT:
            return DataType::Int;
        case GL_UNSIGNED_INT:
            return DataType::UInt;
        case GL_BOOL:
            return DataType::Bool;
        case GL_FLOAT_MAT4:
            return DataType::FloatMat4x4;
        case GL_DOUBLE_MAT4:
            return DataType::DoubleMat4x4;
        case GL_SAMPLER_2D:
            return DataType::Texture2D;
        case GL_SAMPLER_CUBE:
            return DataType::Cubemap;
        default:
            return DataType::Unsupported;
    }
}

void GLShaderProgram::SetShader(const std::string& name, const std::string& source, ShaderType shader_type) {
    // Cannot set the properties that represent the paths of the shaders since we are loading the src directly
    switch(shader_type) {
        case ShaderType::Vertex:
            OnSetShader(name, source, GL_VERTEX_SHADER);
            break;
        case ShaderType::Fragment:
            OnSetShader(name, source, GL_FRAGMENT_SHADER);
            break;
        case ShaderType::Geometry:
            OnSetShader(name, source, GL_GEOMETRY_SHADER);
            break;
    }
}

void GLShaderProgram::OnSetShader(const std::string& path, GLenum GL_shader_type) {
    try {
        std::string source;
        MarkDirty();
        if (path.empty()) {
            /*if (GL_shader_type == GL_VERTEX_SHADER) source = vert_source_;
            else if (GL_shader_type == GL_FRAGMENT_SHADER) source = frag_source_;
            else return;*/
            return;
        } else {
            source = FileIO::ReadTextFile(path);
        }
        OnSetShader(path, source, GL_shader_type);
    } catch (const FileIOException& e) {
        Debug::Log.WriteLine("\"" + GetName() + "\" could not set shader \"" + path + "\"", Priority::Error);
        Debug::Log.WriteLine("    " + std::string(e.what()));
    }
}

void GLShaderProgram::OnSetShader(const std::string& name, const std::string& source, GLenum GL_shader_type) {
    assert(!GLCheckError());
    MarkDirty();
    std::unique_ptr<GLSLShader> shader = std::make_unique<GLSLShader>(source, GL_shader_type);
    // Try to compile the Shader
    try {
        CompileShader(shader->GetID());
    } catch (const ShaderCompileException& e) {
        // If it failed then bail
        Debug::Log.WriteLine("Shader \"" + name + "\" failed to compile", Priority::Error);
        Debug::Log.WriteLine(e.what());
        return;
    }

    attached_shaders_[GL_shader_type] = std::move(shader);
    // Create a new shader program to avoid re-linking issues
    GLuint program = glCreateProgram();
    // Attach all the shaders
    for (auto& kv : attached_shaders_) glAttachShader(program, kv.second->GetID());
    // Bind Attribute Locations
    for (auto& attribute : AttributeLocations()) glBindAttribLocation(program, attribute.second, attribute.first.c_str());
    try {
        // Link the program
       LinkProgram(program);
    } catch (const ProgramLinkException& e) {
        // If it failed then bail
        Debug::Log.WriteLine("Shader program\"" + name + "\" failed to compile", Priority::Error);
        Debug::Log.WriteLine(e.what());
        return;
    }
    if (GLCheckError()) {
        Debug::Log.WriteLine(name + " failed to link", Priority::Error);
        return;
    }
    // Replace the old program with the new one
    glDeleteProgram(program_);
    program_ = program;
    // After compiling, introspect the shading program for uniform information
    QueryUniforms(program_);
    ShaderType type = ShaderType::Fragment;
    if (GL_shader_type == GL_VERTEX_SHADER) type = ShaderType::Vertex;
    else if (GL_shader_type == GL_FRAGMENT_SHADER) type = ShaderType::Fragment;
    else if (GL_shader_type == GL_GEOMETRY_SHADER) type = ShaderType::Geometry;
    shader_texts_[type] = source;
    Changed.Emit();
}

std::vector<std::pair<std::string, DataType>> GLShaderProgram::GetShaderInputs() const {
    return std::vector<std::pair<std::string, DataType>>(uniforms_list_);
}

std::map<std::string, std::pair<GLint, DataType>> GLShaderProgram::GetUniformLocations() const {
    return std::map<std::string, std::pair<GLint, DataType>>(uniform_locations_);
}

GLint GLShaderProgram::GetUniformLocation(const std::string &name) {
    if (uniform_locations_.count(name) == 0) return -1;
    else return uniform_locations_[name].first;
}

void GLShaderProgram::QueryUniforms(GLuint shader_program) {
    uniform_locations_.clear();
    uniforms_list_.clear();

    GLCheckError();
    GLint num_uniforms = 0;
    glGetProgramiv(shader_program, GL_ACTIVE_UNIFORMS, &num_uniforms);
    std::vector<GLchar> name_data(256);
    for (int uniform = 0; uniform < num_uniforms; uniform++) {
        // It's possible that the program is not valid if the user is not done assembling it.
        // In this case, we will just not detect any uniforms.

        // Fetch uniform information
        GLint array_size;
        GLenum gl_type;
        GLsizei name_length;
        glGetActiveUniform(shader_program, uniform, name_data.size(), &name_length, &array_size, &gl_type, &name_data[0]);
        std::string name((char*)&name_data[0], name_length);
        GLint location = glGetUniformLocation(shader_program, &name_data[0]);
        GLCheckError();

        // If it's an array, remove the suffixed [0]
        if (array_size > 1) name = name.substr(0, name.size() - 3);

        // Store uniform information
        DataType type = ConvertType(name, gl_type);
        uniform_locations_[name] = std::make_pair(location, type);
        uniforms_list_.push_back(std::make_pair(name, type));
    }
    assert(uniforms_list_.size() == (unsigned int) num_uniforms);
    assert(uniform_locations_.size() == (unsigned int) num_uniforms);
    GLCheckError();
}

void GLShaderProgram::OnSetVertexShader(std::string path) {
    OnSetShader(path, GL_VERTEX_SHADER);
}
void GLShaderProgram::OnSetFragmentShader(std::string path) {
    OnSetShader(path, GL_FRAGMENT_SHADER);
}
void GLShaderProgram::OnSetGeometryShader(std::string path) {
    OnSetShader(path, GL_GEOMETRY_SHADER);
}

void GLShaderProgram::OnSetTraceCompatible(bool c)
{
    Changed.Emit();
}

// OpenGL 4.3 Required
//void GLShaderProgram::Introspect(GLuint shader_program) {
//    uniform_locations_.clear();
//    uniforms_list_.clear();

//    GLint numUniforms = 0;
//    glGetProgramInterfaceiv(shader_program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
//    const unsigned int NUM_PROPERTIES = 5;
//    const GLenum properties[NUM_PROPERTIES] = {GL_BLOCK_INDEX, GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE};

//    for(int unif = 0; unif < numUniforms; ++unif) {
//        GLint values[NUM_PROPERTIES];
//        glGetProgramResourceiv(shader_program, GL_UNIFORM, unif, NUM_PROPERTIES, properties, NUM_PROPERTIES, NULL, values);

//        // Skip any uniforms that are in a block.
//        if (values[0] != -1) continue;

//        // Get the name of the uniform
//        std::string name(values[1], 0);
//        glGetProgramResourceName(shader_program, GL_UNIFORM, unif, name.size(), NULL, &name[0]);
//        // Get rid of the \000 char at the end of the name and if it is an array,
//        // the name has a suffixed [0] that we should remove
//        if (values[4] > 1) name = name.substr(0, name.size() - 4);
//        else name = name.substr(0, name.size() - 1);

//        // Store the uniform information
//        DataType type = ConvertType(name, values[2]);
//        uniform_locations_[name] = std::make_pair(values[3], type);
//        uniforms_list_.push_back(std::make_pair(name, type));
//    }
//}
