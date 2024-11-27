#version 400

in vec3 position;
in vec3 color;
in vec2 texcoord;

out vec3 Color;
out vec2 Texcoord;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    Color = color;
    Texcoord = texcoord;
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
}
