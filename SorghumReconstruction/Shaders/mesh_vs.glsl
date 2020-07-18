#version 430

layout(location = 0) in vec3 pos_attrib;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;
uniform mat4 PV;
uniform mat4 PVM;

void main()
{
	gl_Position = PVM * vec4(pos_attrib, 1.0);
}
