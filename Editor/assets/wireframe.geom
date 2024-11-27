#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform float screen_width;
uniform float screen_height;

// Distance to each triangle edge
noperspective out vec3 dist;

void main()
{
	// Calculate the triangle properties in screen space
	vec2 screen_size = vec2(screen_width, screen_height);
	vec2 p0 = screen_size * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
	vec2 p1 = screen_size * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
	vec2 p2 = screen_size * gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;
	vec2 v0 = p2 - p1;
	vec2 v1 = p2 - p0;
	vec2 v2 = p1 - p0;
	float area = abs(v1.x * v2.y - v1.y * v2.x);

	// Give each vertex a distance to its opposite edge
	dist = vec3(area / length(v0), 0, 0);
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	dist = vec3(0, area / length(v1), 0);
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	dist = vec3(0, 0, area / length(v2));
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}