#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

in vec3 instance_control_point_0_tcs[];
in vec3 instance_control_point_1_tcs[];
in vec3 instance_control_point_2_tcs[];
in vec3 instance_control_point_3_tcs[];

patch out vec3 instance_control_point_0_tes;
patch out vec3 instance_control_point_1_tes;
patch out vec3 instance_control_point_2_tes;
patch out vec3 instance_control_point_3_tes;

layout(vertices = 2) out;

void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = 1;
		gl_TessLevelOuter[1] = 32;
	}
	
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	// Pass the control points to the tessellation control shader
	instance_control_point_0_tes = instance_control_point_0_tcs[gl_InvocationID];
	instance_control_point_1_tes = instance_control_point_1_tcs[gl_InvocationID];
	instance_control_point_2_tes = instance_control_point_2_tcs[gl_InvocationID];
	instance_control_point_3_tes = instance_control_point_3_tcs[gl_InvocationID];
}
