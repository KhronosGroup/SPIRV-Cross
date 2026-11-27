#version 450
#extension GL_KHR_shader_subgroup_basic : require

layout(triangles) in;
layout(location = 0) flat out uint vLane;

void main()
{
	gl_Position = vec4(gl_TessCoord, 1.0);
	vLane = gl_SubgroupInvocationID;
}
