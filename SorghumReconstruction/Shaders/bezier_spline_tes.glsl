#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

patch in vec3 instance_control_point_0_tes;
patch in vec3 instance_control_point_1_tes;
patch in vec3 instance_control_point_2_tes;
patch in vec3 instance_control_point_3_tes;

out vec3 position_model;
out vec3 position_world;

layout (isolines, fractional_odd_spacing, ccw) in;

vec4 BezierCurve(const float u)
{
	const float bu0 = (1.0 - u) * (1.0 - u) * (1.0 - u);
	const float bu1 = 3.0 * u * (1.0 - u) * (1.0 - u);
	const float bu2 = 3.0 * u * u * (1.0 - u);
	const float bu3 = u * u * u;

	vec3 p = instance_control_point_0_tes * bu0
	       + instance_control_point_1_tes * bu1
		   + instance_control_point_2_tes * bu2
		   + instance_control_point_3_tes * bu3;

	return vec4(p, 1.0);
}

void main()
{
	const float u = gl_TessCoord.x;
	const float v = gl_TessCoord.y;

	const vec4 p0 = gl_in[0].gl_Position;
	const vec4 p1 = gl_in[1].gl_Position;

	const vec4 a = mix(p0, p1, u);

	// Evaluate patch
	const vec4 p = BezierCurve(a.x);

	gl_Position = PVM * p;

	position_model = vec3(p);
	position_world = vec3(M * p);
}
