#version 400

in vec3 Color;
in vec2 Texcoord;

out vec4 frag_color;

uniform sampler2D DiffuseMap;

void main()
{
	frag_color = texture(DiffuseMap, Texcoord);
}
