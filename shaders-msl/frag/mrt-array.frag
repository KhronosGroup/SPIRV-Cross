#version 310 es
precision mediump float;

layout(location = 0) out vec4 FragColor[4];
layout(location = 0) in vec4 vA;
layout(location = 1) in vec4 vB;

void main()
{
	FragColor[0] = mod(vA, vB);
	FragColor[1] = vA + vB;
	FragColor[2] = vA - vB;
	FragColor[3] = vA * vB;
}
