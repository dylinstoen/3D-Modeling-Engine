#version 400
// TODO: Fix triangles with one vertex behind camera:
// http://developer.download.nvidia.com/SDK/10/direct3d/Source/SolidWireframe/Doc/SolidWireframe.pdf
// TODO: Re-outline silhouette edges, since this method only outlines inside edges of triangles

noperspective in vec3 dist;

out vec4 frag_color;

void main()
{
	float min_dist = min(min(dist[0], dist[1]), dist[2]);
	float intensity = exp2(-2.0 * min_dist * min_dist);

	if (min_dist > 3.0f) 
		discard;
	else if (min_dist > 1.0f)
		frag_color = intensity * vec4(0.0f, 1.0f, 0.0f, 1.0) + (1 - intensity) * vec4(0.0f, 0.0f, 0.0f, 0.0f);
	else
		frag_color = vec4(0.0f, 1.0f, 0.0f, 1.0);

	// Offset in order to pass the depth buffer test
	// TODO: Find a better way to do this? Causes some artefacts
	gl_FragDepth = gl_FragCoord.z*0.999;
}
