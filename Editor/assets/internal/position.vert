#version 400

in vec3 position;
out vec4 interpolated_position;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

void main()
{
    // Outputs camera-space position
    interpolated_position = view_matrix * model_matrix * vec4(position, 1.0);
    gl_Position = projection_matrix * interpolated_position;
}
