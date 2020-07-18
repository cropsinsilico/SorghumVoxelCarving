#version 430

in vec2 uv;

out vec4 fragColor;

uniform sampler2D image;

void main()
{
	fragColor = vec4(texture(image, uv).rgb, 0.5);
}
