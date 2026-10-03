#version 450
layout(triangles, equal_spacing, ccw) in;

layout(location = 0) in vec2 vUV[];
layout(location = 0) out vec2 oUV;

void main()
{
	oUV = vUV[0] * gl_TessCoord.x + vUV[1] * gl_TessCoord.y + vUV[2] * gl_TessCoord.z;
	gl_Position = gl_in[0].gl_Position * gl_TessCoord.x +
	              gl_in[1].gl_Position * gl_TessCoord.y +
	              gl_in[2].gl_Position * gl_TessCoord.z;
}
