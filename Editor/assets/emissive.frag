#version 400

// Interpolated inputs from Vertex Shader
in vec3 world_normal;
in vec3 world_vertex;
in vec3 world_eye;
in vec2 UV;

// Final color of this fragment
out vec4 frag_color;

// Built-in Variables
uniform vec3 point_light_ambient[4];
uniform vec3 point_light_intensity[4];
uniform vec3 point_light_position[4];
uniform float point_light_atten_quad[4];
uniform float point_light_atten_linear[4];
uniform float point_light_atten_const[4];
uniform vec3 dir_light_ambient[4];
uniform vec3 dir_light_intensity[4];
uniform vec3 dir_light_direction[4];

// User-defined Variables
uniform vec3 Color;
uniform vec3 AmbientColor;
uniform float ConstantAttenuation;
uniform float LinearAttenuation;
uniform float QuadraticAttenuation;

void main() {
	// Assign final color
	float dummy = 1.f + 0.000001f*(ConstantAttenuation + LinearAttenuation + QuadraticAttenuation);
	frag_color = vec4((Color+AmbientColor)*dummy, 1.0);
}
