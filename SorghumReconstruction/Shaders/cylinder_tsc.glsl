#version 430

layout(vertices = 4) out; // number of output verts of the tess. control shader

void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = 6;
		gl_TessLevelOuter[1] = 6;
		gl_TessLevelOuter[2] = 6;
		gl_TessLevelOuter[3] = 6;
		gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[2], 0.5);
		gl_TessLevelInner[1] = mix(gl_TessLevelOuter[1], gl_TessLevelOuter[3], 0.5);
	}
	
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}