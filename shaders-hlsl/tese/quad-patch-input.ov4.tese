#version 450
layout(quads, fractional_odd_spacing, cw) in;

layout(location = 0) in vec4 vPos[];
layout(location = 1) patch in vec4 vColor;
layout(location = 0) out vec4 oColor;

void main()
{
	vec4 a = mix(vPos[0], vPos[1], gl_TessCoord.x);
	vec4 b = mix(vPos[3], vPos[2], gl_TessCoord.x);
	gl_Position = mix(a, b, gl_TessCoord.y);
	oColor = vColor * (gl_TessLevelOuter[0] + gl_TessLevelInner[1]) +
	         vec4(float(gl_PrimitiveID), float(gl_PatchVerticesIn), 0.0, 0.0);
}
