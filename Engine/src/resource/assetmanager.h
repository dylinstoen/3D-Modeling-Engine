/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <animator.h>

#include <singleton.h>

class ShaderFactory;
class Asset;
class Mesh;
class Material;
class ShaderProgram;
class Texture;
class Cubemap;

// AssetManager owns all the Scene's assets and caches them for reference via names.
// Assets not created through the AssetManager will not be kept track of.
// Asset names are unique for each type of Asset.
class AssetManager : public Singleton<AssetManager> {
public:
    AssetManager(ShaderFactory &shader_factory);
    ~AssetManager();

    // Load from Disk. If failed, has no effect and writes a log message.
    // If the name already exists, replaces the pre-existing asset.
    void LoadTexture(const std::string& name, const std::string& path);
    void LoadCubemap(const std::string& name, const std::string& path);
    void LoadMesh(const std::string& name, const std::string& path, bool internal=false);

    // Creates an asset in-memory. If the name already exists, returns a nullptr.
    // Internal indicates this asset is not to be serialized (as it is created in the code).
    Mesh* CreateMesh(const std::string& name, MeshType meshtype = MeshType::Triangles, bool internal = true, bool hidden = true); // Returns an empty Mesh
    Material* CreateMaterial(const std::string& name, bool internal = true); // Returns a material using the default shading program
    ShaderProgram* CreateShaderProgram(const std::string& name, bool internal = true); // Returns a shading program with default vertex and fragment shaders

    // Unload from Memory
    void UnloadTexture(const std::string& name);
    void UnloadCubemap(const std::string& name);
    void UnloadMaterial(const std::string& name);
    void UnloadMesh(const std::string& name);
    void UnloadShaderProgram(const std::string& name);

    // Cache accessors
    Texture* GetTexture(const std::string& name);
    Cubemap* GetCubemap(const std::string& name);
    Material* GetMaterial(const std::string& name);
    Mesh* GetMesh(const std::string& name);
    ShaderProgram* GetShaderProgram(const std::string& name);

    Texture* GetOrCreateSolidTexture(glm::vec3 color);

    void Refresh();  // Reloads all assets from disk

    // Signals
    Signal1<Asset&> AssetCreated;
    Signal1<uint64_t> AssetDeleted;

    // Used for serialization and UI
    std::vector<Texture*> GetTextures() const;
    std::vector<Cubemap*> GetCubemaps() const;
    std::vector<Material*> GetMaterials() const;
    std::vector<Mesh*> GetMeshes() const;
    std::vector<ShaderProgram*> GetShaderPrograms() const;

    ShaderProgram& GetDefaultShaderProgram() const { return *default_shader_; }
    Material& GetDefaultMaterial() const { return *default_material_; }
    Texture& GetDefaultTexture() const { return *default_texture_; }
    Cubemap& GetDefaultCubemap() const { return *default_cubemap_; }

protected:
    const std::string depth_frag_src =
        "#version 400\n"
        "out vec4 frag_color;"
        "in vec4 interpolated_position;"
        "void main() {"
            "float depth = length(interpolated_position.xyz);"
            "frag_color = vec4(depth/10.f, depth/10.f, depth/10.f, 1.f);"
        "}";

    const std::string blinn_phong_vert_src_ =
        "#version 400\n"
        "in vec3 position;"
        "in vec3 normal;"
        "in vec2 texcoord;"
        "out vec3 world_normal;"
        "out vec3 world_vertex;"
        "out vec3 world_eye;"
        "out vec2 UV;"
        "uniform mat4 model_matrix;"
        "uniform mat4 view_matrix;"
        "uniform mat4 projection_matrix;"
        "void main() {"
        "	mat4 modelview_matrix = view_matrix * model_matrix;"
        "   mat3 normal_matrix = transpose(inverse(mat3(model_matrix)));"
        "	world_vertex = vec3(model_matrix * vec4(position, 1.0));"
        "   world_normal = normalize(normal_matrix * normal);"
        "	world_eye = vec3(inverse(view_matrix) * vec4(0.f,0.f,0.f,1.f));"
        "	UV = texcoord;"
        "	gl_Position = projection_matrix * modelview_matrix * vec4(position, 1.0);"
        "}";

    const std::string blinn_phong_frag_src_ =
        "#version 400\n"
        "in vec3 world_normal;"
        "in vec3 world_vertex;"
        "in vec3 world_eye;"
        "in vec2 UV;"
        "out vec4 frag_color;"
        "uniform vec3 point_light_ambient[4];"
        "uniform vec3 point_light_intensity[4];"
        "uniform vec3 point_light_position[4];"
        "uniform float point_light_atten_quad[4];"
        "uniform float point_light_atten_linear[4];"
        "uniform float point_light_atten_const[4];"
        "uniform vec3 dir_light_ambient[4];"
        "uniform vec3 dir_light_intensity[4];"
        "uniform vec3 dir_light_direction[4];"
        "uniform vec3 spot_light_ambient[4];"
        "uniform vec3 spot_light_direction[4];"
        "uniform vec3 spot_light_intensity[4];"
        "uniform vec3 spot_light_position[4];"
        "uniform float spot_light_atten_quad[4];"
        "uniform float spot_light_atten_linear[4];"
        "uniform float spot_light_atten_const[4];"
        "uniform float spot_light_cutoff[4];"
        "uniform float spot_light_falloff[4];"
        "uniform vec3 area_light_intensity[4];"
        "uniform vec3 area_light_ambient[4];"
        "uniform vec3 area_light_position[4];"
        "uniform float area_light_atten_quad[4];"
        "uniform float area_light_atten_linear[4];"
        "uniform float area_light_atten_const[4];"
        "uniform sampler2D Emissive;"
        "uniform sampler2D Diffuse;"
        "uniform sampler2D Specular;"
        "uniform float Shininess;"
        "vec3 EmissiveColor;"
        "vec3 DiffuseColor;"
        "vec3 SpecularColor;"
        "vec3 DirLightContribution(int light_num) {"
        "	vec3 N = normalize(world_normal);"
        "	vec3 V = normalize(world_eye - world_vertex);"
        "	vec3 L = -normalize(dir_light_direction[light_num]);"
        "	vec3 H = normalize(V+L);"
        "	float B = 1.0;"
        "	if (dot(N, L) < 0.00001) { B = 0.0; }"
        "	float diffuseShade = max(dot(N, L), 0.0);"
        "	float shininess = Shininess > 0 ? Shininess : 0.00001;"
        "	float specularShade = B * pow(max(dot(H, N), 0.0), shininess);"
        "	vec3 ambient = DiffuseColor * dir_light_ambient[light_num];"
        "	vec3 diffuse = diffuseShade * DiffuseColor * dir_light_intensity[light_num];"
        "	vec3 specular = specularShade * SpecularColor * dir_light_intensity[light_num];"
        "	return ambient + diffuse + specular;"
        "}"
        "vec3 PointLightContribution(int light_num) {"
        "	vec3 N = normalize(world_normal);"
        "	vec3 V = normalize(world_eye - world_vertex);"
        "	vec3 light_vector = point_light_position[light_num] - world_vertex;"
        "	vec3 L = normalize(light_vector);"
        "	vec3 H = normalize(V+L);"
        "	float r = length(light_vector);"
        "	float attenuation = point_light_atten_const[light_num] + point_light_atten_linear[light_num] * r + point_light_atten_quad[light_num] * r * r;"
        "	float luminosity = 0;"
        "	if (attenuation > 0) { luminosity = 1.0 / attenuation; }"
        "	float B = 1.0;"
        "	if (dot(N, L) < 0.00001) { B = 0.0; }"
        "	float diffuseShade = max(dot(N, L), 0.0);"
        "	float shininess = Shininess > 0 ? Shininess : 0.00001;"
        "	float specularShade = B * pow(max(dot(H, N), 0.0), shininess);"
        "	vec3 ambient = DiffuseColor * point_light_ambient[light_num];"
        "	vec3 diffuse = diffuseShade * DiffuseColor * point_light_intensity[light_num] * luminosity;"
        "	vec3 specular = specularShade * SpecularColor * point_light_intensity[light_num] * luminosity;"
        "	return ambient + diffuse + specular;"
        "}"
        "vec3 SpotLightContribution(int light_num) {"
        "	vec3 ambient = DiffuseColor * spot_light_ambient[light_num];"
        "   if (spot_light_direction[light_num] == vec3(0, 0, 0))"
        "       return ambient;"
        "	vec3 N = normalize(world_normal);"
        "	vec3 V = normalize(world_eye - world_vertex);"
        "	vec3 light_vector = spot_light_position[light_num] - world_vertex;"
        "	vec3 L = normalize(light_vector);"
        "	vec3 S = -normalize(spot_light_direction[light_num]);"
        "	vec3 H = normalize(V+L);"
        "	float r = length(light_vector);"
        "	float attenuation = spot_light_atten_const[light_num] + spot_light_atten_linear[light_num] * r + spot_light_atten_quad[light_num] * r * r;"
        "	float luminosity = 0;"
        "	if (attenuation > 0) { luminosity = 1.0 / attenuation; }"
        "   float LdotS = dot(L, S);"
        "	float B = 1.0;"
        "	if (dot(N, L) < 0.00001 || LdotS < cos(spot_light_cutoff[light_num] / 180.0 * 3.14159)) { B = 0.0; }"
        "	float diffuseShade = B * max(dot(N, L), 0.0);"
        "	float shininess = Shininess > 0 ? Shininess : 0.00001;"
        "	float specularShade = B * pow(max(dot(H, N), 0.0), shininess);"
        "	vec3 diffuse = diffuseShade * DiffuseColor * spot_light_intensity[light_num] * pow(LdotS, spot_light_falloff[light_num]) * luminosity;"
        "	vec3 specular = specularShade * SpecularColor * spot_light_intensity[light_num] * pow(LdotS, spot_light_falloff[light_num]) * luminosity;"
        "	return ambient + diffuse + specular;"
        "}"
        "vec3 AreaLightContribution(int light_num) {"
        "	vec3 N = normalize(world_normal);"
        "	vec3 V = normalize(world_eye - world_vertex);"
        "	vec3 light_vector = area_light_position[light_num] - world_vertex;"
        "	vec3 L = normalize(light_vector);"
        "	vec3 H = normalize(V+L);"
        "	float r = length(light_vector);"
        "	float attenuation = 1 + area_light_atten_const[light_num] + area_light_atten_linear[light_num] * r + area_light_atten_quad[light_num] * r * r;"
        "	float luminosity = 0;"
        "	if (attenuation > 0) { luminosity = 1.0 / attenuation; }"
        "	float B = 1.0;"
        "	if (dot(N, L) < 0.00001) { B = 0.0; }"
        "	float diffuseShade = max(dot(N, L), 0.0);"
        "	float shininess = Shininess > 0 ? Shininess : 0.00001;"
        "	float specularShade = B * pow(max(dot(H, N), 0.0), shininess);"
        "	vec3 ambient = DiffuseColor * area_light_ambient[light_num];"
        "	vec3 diffuse = diffuseShade * DiffuseColor * area_light_intensity[light_num] * luminosity;"
        "	vec3 specular = specularShade * SpecularColor * area_light_intensity[light_num] * luminosity;"
        "	return ambient + diffuse + specular;"
        "}"
        "void main() {"
        "   EmissiveColor = texture(Emissive, UV).xyz;"
        "   DiffuseColor = texture(Diffuse, UV).xyz;"
        "   SpecularColor = texture(Specular, UV).xyz;"
        "	vec3 dirlight_contribution = vec3(0, 0, 0);"
        "	for (int i = 0; i < 4; i++) dirlight_contribution += DirLightContribution(i);"
        "	vec3 pointlight_contribution = vec3(0, 0, 0);"
        "	for (int i = 0; i < 4; i++) pointlight_contribution += PointLightContribution(i);"
        "	vec3 spotlight_contribution = vec3(0, 0, 0);"
        "	for (int i = 0; i < 4; i++) spotlight_contribution += SpotLightContribution(i);"
        "   vec3 arealight_contribution = vec3(0, 0, 0);"
        "   for (int i = 0; i < 4; i++) arealight_contribution += AreaLightContribution(i);"
        "	frag_color = vec4(dirlight_contribution + pointlight_contribution + spotlight_contribution + arealight_contribution + EmissiveColor, 1.0);"
        "}";

    const std::string alpha_frag_src_ =
        "#version 400\n"
        "in vec3 Color;"
        "in vec2 Texcoord;"
        "out vec4 frag_color;"
        "uniform sampler2D DiffuseMap;"
        "void main()"
        "{"
        "vec4 tex_color = texture(DiffuseMap, Texcoord);"
        "if (tex_color.w < 0.5) { discard; }"
        "frag_color = tex_color;"
        "}";

    const std::string toon_frag_src_ =
        "#version 400\n"
        "in vec3 world_normal;"
        "in vec3 world_vertex;"
        "in vec3 world_eye;"
        "in vec2 UV;"
        "out vec4 frag_color;"
        "uniform vec3 point_light_ambient[4];"
        "uniform vec3 point_light_intensity[4];"
        "uniform vec3 point_light_position[4];"
        "uniform float point_light_atten_quad[4];"
        "uniform float point_light_atten_linear[4];"
        "uniform float point_light_atten_const[4];"
        "uniform samplerCube point_light_shadowmap[4];"
        "uniform vec3 dir_light_ambient[4];"
        "uniform vec3 dir_light_intensity[4];"
        "uniform vec3 dir_light_direction[4];"
        "uniform sampler2D Emissive;"
        "uniform sampler2D Diffuse;"
        "uniform sampler2D Specular;"
        "uniform float Shininess;"
        "vec3 EmissiveColor;"
        "vec3 DiffuseColor;"
        "vec3 SpecularColor;"
        "void main()"
        "{"
            "EmissiveColor = texture(Emissive, UV).xyz;"
            "DiffuseColor = texture(Diffuse, UV).xyz;"
            "SpecularColor = texture(Specular, UV).xyz;"
            "vec3 N = normalize(world_normal);"
            "vec3 V = normalize(world_eye - world_vertex);"
            "float shininess = Shininess > 0 ? Shininess : 0.00001;"
            "vec3 lighting = vec3(0, 0, 0);"
            "for (int i = 0; i < 4; i++) {"
                "vec3 L = -normalize(dir_light_direction[i]);"
                "vec3 H = normalize(V+L);"
                "float B = 1.0;"
                "if (dot(N, L) < 0.00001) { B = 0.0; }"
                "float intensity = max(dot(N, L), 0.0);"
                "float s_intensity = B * pow(max(dot(H, N), 0.0), shininess);"
                "vec3 color = DiffuseColor * dir_light_ambient[i] + DiffuseColor * dir_light_intensity[i];"
                "if (intensity > 0.95) { color *= vec3(5); }"
                "else if (intensity > 0.8) { color *= vec3(2); }"
                "else if (intensity > 0.5) { color *= vec3(.95); }"
                "else { color *= vec3(0.5); }"
                "if (s_intensity > 0.95) { color += SpecularColor * s_intensity; }"
                "if (dot(N, V) < 0.3) { color = vec3(0); }"
                "lighting += color;"
                "vec3 light_vector = point_light_position[i] - world_vertex;"
                "L = normalize(light_vector);"
                "H = normalize(V+L);"
                "B = 1.0;"
                "if (dot(N, L) < 0.00001) { B = 0.0; }"
                "float r = length(light_vector);"
                "float atten = point_light_atten_const[i] + point_light_atten_linear[i] * r + point_light_atten_quad[i] * r * r;"
                "float luminosity = 0;"
                "if (atten > 0) { luminosity = 1.0 / atten; }"
                "intensity = max(dot(N, L), 0.0) * luminosity;"
                "s_intensity = B * pow(max(dot(H, N), 0.0), shininess) * luminosity;"
                "color = DiffuseColor * point_light_ambient[i] + DiffuseColor * point_light_intensity[i];"
                "if (intensity > 0.95) { color *= vec3(5); }"
                "else if (intensity > 0.8) { color *= vec3(2); }"
                "else if (intensity > 0.5) { color *= vec3(.95); }"
                "else { color *= vec3(0.5); }"
                "if (s_intensity > 0.95) { color += SpecularColor * s_intensity; }"
                "if (dot(N, V) < 0.3) { color = vec3(0); }"
                "lighting += color;"
            "}"
            "frag_color = vec4(lighting + EmissiveColor, 1.0);"
        "}";

    ShaderFactory* shader_factory_;

    // Default assets that serve as fallbacks in case the user asset is not valid
    // TODO: Default mesh?
    std::unique_ptr<Texture> default_texture_;
    std::unique_ptr<Cubemap> default_cubemap_;
    std::unique_ptr<ShaderProgram> default_shader_;
    std::unique_ptr<Material> default_material_;

    // Resource Cache
    // TODO: Potential memory issue here. Material depends on reference to ShaderProgram, but ShaderProgram can be unloaded separately from Material, which invalidates Material's pointer.
    // Same issue with like Material uses Texture etc. Unless we guarantee that they won't be unloaded until the very end, or some other way to deal with this.
    std::map<std::string, std::unique_ptr<Texture>> textures_;
    std::map<std::string, std::unique_ptr<Cubemap>> cubemaps_;
    std::map<std::string, std::unique_ptr<Material>> materials_;
    std::map<std::string, std::unique_ptr<Mesh>> meshes_;
    std::map<std::string, std::unique_ptr<ShaderProgram>> shader_programs_;

    std::map<unsigned int, std::unique_ptr<Texture>> solid_textures_;
};

#endif // ASSETMANAGER_H
