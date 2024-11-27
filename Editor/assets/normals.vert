#version 400

in vec3 position;
in vec3 normal;

out vec4 exploded;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform float NormalLength;

void main()
{
	mat4 modelview_matrix = view_matrix * model_matrix;
	mat3 normal_matrix = mat3(transpose(inverse(modelview_matrix)));
	
	exploded = projection_matrix * modelview_matrix * vec4(position + normal * NormalLength, 1.0);

	// Always have to transform vertex positions so they end
	// up in the right place on the screen.
	gl_Position = projection_matrix * modelview_matrix * vec4(position, 1.0);
}