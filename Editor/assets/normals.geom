#version 400

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

uniform vec3 NormalColor;

in vec4 exploded[];

out vec4 vertex_color;

void main()
{

	for (int i = 0; i < gl_in.length(); i++) {
		vertex_color = vec4(NormalColor, 1.0);
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();

		vertex_color = vec4(NormalColor, 1.0);
		gl_Position = exploded[i];
		EmitVertex();

		EndPrimitive();
	}
}
