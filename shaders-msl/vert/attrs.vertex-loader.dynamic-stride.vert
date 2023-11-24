#version 310 es

layout (location = 0) in vec4 a0;
layout (location = 1) in vec4 a1;
layout (location = 3) in vec4 a3;
layout (location = 4) in vec4 a4;
layout (location = 5) in vec4 a5;
layout (location = 6) in vec4 a6;
layout (location = 7) in uint a7;
layout (location = 8) in vec4 a8;

void main()
{
	gl_Position = a0 + a1 + a3 + a4 + a5 + a6 + float(a7) + a8;
}
