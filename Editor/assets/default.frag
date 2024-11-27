#version 400

out vec4 frag_color;

uniform vec3 Color;

void main()
{
    frag_color = vec4(Color, 1.0);
}
