#version 430

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

layout (quads, fractional_odd_spacing, ccw) in;

out vec3 position_model;
out vec3 position_world;
out vec3 normal_world;

vec3 normal(const vec4 p)
{
	if (p.z >= 0.5)
	{
		return vec3(0.0, 0.0, 1.0);
	}
	else if (p.z <= -0.5)
	{
		return vec3(0.0, 0.0, -1.0);
	}
	else
	{
		return vec3(p.xy, 0.0);
	}
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

	vec4 p = mix(a, b, v);

	p.xy = normalize(p.xy);

	gl_Position = PVM * p;

	position_model = vec3(p);
	position_world = vec3(M * p);
	normal_world = normalize(N * normal(p));
}