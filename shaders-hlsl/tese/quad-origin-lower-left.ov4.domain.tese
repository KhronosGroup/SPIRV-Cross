#version 450
layout(quads, equal_spacing, ccw) in;

layout(location = 0) in vec4 vPos[];

void main()
{
	vec4 a = mix(vPos[0], vPos[1], gl_TessCoord.x);
	vec4 b = mix(vPos[3], vPos[2], gl_TessCoord.x);
	gl_Position = mix(a, b, gl_TessCoord.y);
}
