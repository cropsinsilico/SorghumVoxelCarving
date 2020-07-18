#version 430

in vec3 position_model;
in vec3 position_world;
in vec3 normal_world;

out vec4 fragColor;

void main()
{
	const vec3 light_direction = normalize(vec3(1.0, -1.0, 2.0));

	const float light = max(0.0, dot(light_direction, normal_world));

	const float color = 0.25 + 0.75 * light;

	fragColor = vec4(color, color, color, 1.0);
}