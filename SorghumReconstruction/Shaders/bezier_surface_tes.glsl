#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

patch in mat4 instance_point_x_tes;
patch in mat4 instance_point_y_tes;
patch in mat4 instance_point_z_tes;

out vec3 position_model;
out vec3 position_world;
out vec3 normal_world;

layout (quads, fractional_odd_spacing, ccw) in;

vec3 control_point(int i, int j)
{
	return vec3(instance_point_x_tes[i][j], instance_point_y_tes[i][j], instance_point_z_tes[i][j]);
}

vec4 BezierPatch(const float u, const float v)
{
	const float bu0 = (1.0 - u) * (1.0 - u) * (1.0 - u);
	const float bu1 = 3.0 * u * (1.0 - u) * (1.0 - u);
	const float bu2 = 3.0 * u * u * (1.0 - u);
	const float bu3 = u * u * u;

	const float bv0 = (1.0 - v) * (1.0 - v) * (1.0 - v);
	const float bv1 = 3.0 * v * (1.0 - v) * (1.0 - v);
	const float bv2 = 3.0 * v * v * (1.0 - v);
	const float bv3 = v * v * v;

	vec3 p = control_point(0, 0) * bu0 * bv0
	       + control_point(0, 1) * bu0 * bv1
		   + control_point(0, 2) * bu0 * bv2
		   + control_point(0, 3) * bu0 * bv3
		   + control_point(1, 0) * bu1 * bv0
		   + control_point(1, 1) * bu1 * bv1
		   + control_point(1, 2) * bu1 * bv2
		   + control_point(1, 3) * bu1 * bv3
		   + control_point(2, 0) * bu2 * bv0
		   + control_point(2, 1) * bu2 * bv1
		   + control_point(2, 2) * bu2 * bv2
		   + control_point(2, 3) * bu2 * bv3
		   + control_point(3, 0) * bu3 * bv0
		   + control_point(3, 1) * bu3 * bv1
		   + control_point(3, 2) * bu3 * bv2
		   + control_point(3, 3) * bu3 * bv3;

	return vec4(p, 1.0);
}

vec3 BezierPatchNormal(const float u, const float v)
{
	const float bu0 = (1.0 - u) * (1.0 - u) * (1.0 - u);
	const float bu1 = 3.0 * u * (1.0 - u) * (1.0 - u);
	const float bu2 = 3.0 * u * u * (1.0 - u);
	const float bu3 = u * u * u;

	const float bv0 = (1.0 - v) * (1.0 - v) * (1.0 - v);
	const float bv1 = 3.0 * v * (1.0 - v) * (1.0 - v);
	const float bv2 = 3.0 * v * v * (1.0 - v);
	const float bv3 = v * v * v;

	const float dbu0 = -3.0 * (1.0 - u) * (1.0 - u);
	const float dbu1 = 3.0 * (1.0 - u) * (1.0 - u) - 6.0 * u * (1.0 - u);
	const float dbu2 = 6.0 * u * (1.0 - u) - 3.0 * u * u;
	const float dbu3 = 3.0 * u * u;

	const float dvu0 = -3.0 * (1.0 - v) * (1.0 - v);
	const float dvu1 = 3.0 * (1.0 - v) * (1.0 - v) - 6.0 * v * (1.0 - v);
	const float dvu2 = 6.0 * v * (1.0 - v) - 3.0 * v * v;
	const float dvu3 = 3.0 * v * v;
	
	// Partial derivative with respect to u
	const vec3 du = dbu0 * bv0 * control_point(0, 0)
	              + dbu0 * bv1 * control_point(0, 1)
	              + dbu0 * bv2 * control_point(0, 2)
	              + dbu0 * bv3 * control_point(0, 3)
				  + dbu1 * bv0 * control_point(1, 0)
				  + dbu1 * bv1 * control_point(1, 1)
				  + dbu1 * bv2 * control_point(1, 2)
				  + dbu1 * bv3 * control_point(1, 3)
				  + dbu2 * bv0 * control_point(2, 0)
				  + dbu2 * bv1 * control_point(2, 1)
				  + dbu2 * bv2 * control_point(2, 2)
				  + dbu2 * bv3 * control_point(2, 3)
				  + dbu3 * bv0 * control_point(3, 0)
				  + dbu3 * bv1 * control_point(3, 1)
				  + dbu3 * bv2 * control_point(3, 2)
				  + dbu3 * bv3 * control_point(3, 3);

	// Partial derivative with respect to v
	const vec3 dv = dvu0 * bu0 * control_point(0, 0)
	              + dvu0 * bu1 * control_point(1, 0)
	              + dvu0 * bu2 * control_point(2, 0)
	              + dvu0 * bu3 * control_point(3, 0)
				  + dvu1 * bu0 * control_point(0, 1)
				  + dvu1 * bu1 * control_point(1, 1)
				  + dvu1 * bu2 * control_point(2, 1)
				  + dvu1 * bu3 * control_point(3, 1)
				  + dvu2 * bu0 * control_point(0, 2)
				  + dvu2 * bu1 * control_point(1, 2)
				  + dvu2 * bu2 * control_point(2, 2)
				  + dvu2 * bu3 * control_point(3, 2)
				  + dvu3 * bu0 * control_point(0, 3)
				  + dvu3 * bu1 * control_point(1, 3)
				  + dvu3 * bu2 * control_point(2, 3)
				  + dvu3 * bu3 * control_point(3, 3);

	return normalize(cross(du, dv));
}

void main()
{
	const float u = gl_TessCoord.x;
	const float v = gl_TessCoord.y;

	const vec4 p0 = gl_in[0].gl_Position;
	const vec4 p1 = gl_in[1].gl_Position;
	const vec4 p2 = gl_in[2].gl_Position;
	const vec4 p3 = gl_in[3].gl_Position;

	const vec4 a = mix(p0, p1, u);
	const vec4 b = mix(p3, p2, u);

	// 2D coordinates in the patch space
	vec2 uv = vec2(mix(a, b, v));

	// Evaluate patch
	vec4 p = BezierPatch(uv.s, uv.t);

	gl_Position = PVM * p;

	position_model = vec3(p);
	position_world = vec3(M * p);
	normal_world = normalize(N * BezierPatchNormal(uv.s, uv.t));
}
