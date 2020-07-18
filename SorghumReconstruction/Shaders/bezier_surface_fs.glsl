#version 430

in vec3 position_model;
in vec3 position_world;
in vec3 normal_world;

out vec4 fragColor;

void main()
{
	vec3 color = 0.5 * normal_world + 0.5;

	fragColor = vec4(color, 1.0);
}
