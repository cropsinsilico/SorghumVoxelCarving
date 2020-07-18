#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

in mat4 instance_point_x_tcs[];
in mat4 instance_point_y_tcs[];
in mat4 instance_point_z_tcs[];

patch out mat4 instance_point_x_tes;
patch out mat4 instance_point_y_tes;
patch out mat4 instance_point_z_tes;

layout(vertices = 4) out;

void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = 9;
		gl_TessLevelOuter[1] = 9;
		gl_TessLevelOuter[2] = 9;
		gl_TessLevelOuter[3] = 9;
		gl_TessLevelInner[0] = 9;
		gl_TessLevelInner[1] = 9;
	}
	
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	// Pass the control points to the tessellation control shader
	instance_point_x_tes = instance_point_x_tcs[gl_InvocationID];
	instance_point_y_tes = instance_point_y_tcs[gl_InvocationID];
	instance_point_z_tes = instance_point_z_tcs[gl_InvocationID];
}
