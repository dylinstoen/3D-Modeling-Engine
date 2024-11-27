#version 400

in vec3 position;
in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
	// vec3 outset_pos = position + normal * 0.01;
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
}