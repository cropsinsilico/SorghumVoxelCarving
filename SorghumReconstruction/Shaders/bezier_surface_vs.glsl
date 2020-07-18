#version 430

layout(location = 0) in vec3 pos_attrib;

layout(location = 1) in mat4 instance_point_x_attrib;
layout(location = 5) in mat4 instance_point_y_attrib;
layout(location = 9) in mat4 instance_point_z_attrib;

out mat4 instance_point_x_tcs;
out mat4 instance_point_y_tcs;
out mat4 instance_point_z_tcs;

void main()
{
	gl_Position = vec4(pos_attrib, 1.0);

	// Pass the control points to the tessellation control shader
	instance_point_x_tcs = instance_point_x_attrib;
	instance_point_y_tcs = instance_point_y_attrib;
	instance_point_z_tcs = instance_point_z_attrib;
}
