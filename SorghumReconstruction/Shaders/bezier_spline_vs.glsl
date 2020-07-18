#version 430

layout(location = 0) in vec3 pos_attrib;

layout(location = 1) in vec3 instance_control_point_0_attrib;
layout(location = 2) in vec3 instance_control_point_1_attrib;
layout(location = 3) in vec3 instance_control_point_2_attrib;
layout(location = 4) in vec3 instance_control_point_3_attrib;

out vec3 instance_control_point_0_tcs;
out vec3 instance_control_point_1_tcs;
out vec3 instance_control_point_2_tcs;
out vec3 instance_control_point_3_tcs;

void main()
{
	gl_Position = vec4(pos_attrib, 1.0);

	// Pass the control points to the tessellation control shader
	instance_control_point_0_tcs = instance_control_point_0_attrib;
	instance_control_point_1_tcs = instance_control_point_1_attrib;
	instance_control_point_2_tcs = instance_control_point_2_attrib;
	instance_control_point_3_tcs = instance_control_point_3_attrib;
}
