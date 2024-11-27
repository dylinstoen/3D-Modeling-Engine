/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef ENUM_H
#define ENUM_H

enum class AssetType {
    Texture,
    RenderableTexture,
    Cubemap,
    RenderableCubemap,
    Mesh,
    Material,
    ShaderProgram
};

enum class MeshType {
    Triangles,
    Quads,
    Lines
};

enum class ShaderType {
    Vertex,
    Fragment,
    Geometry
};

enum class PrimitiveType {
    Cube,
    Sphere,
    Cylinder,
    Cone,
    Plane
};

enum class DataType {
    Float,
    Float3,
    Float4,
    FloatMat4x4,
    Double,
    Double3,
    Double4,
    DoubleMat4x4,
    Int,
    UInt,
    Bool,
    Texture2D,
    ColorRGB,
    ColorRGBA,
    Cubemap,
    Unsupported
};

enum class CurveType {
    Linear,
    Bezier,
    BSpline,
    CatmullRom
};

// Maybe in the future we should have this be a class instead
enum class FileType {
    VertexShader,
    FragmentShader,
    GeometryShader,
    Curve,
    Image,
    Mesh,
    Scene,
    Points,
    Ray
};

enum class Space {
    World,
    Local
};

enum class RenderingMode {
    Shaded,
    Wireframe,
    Points
};

enum class Priority {
    Normal,
    Status,
    Warning,
    Error
};

// Required when using enum class as unordered_map key
struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

#endif // ENUM_H
