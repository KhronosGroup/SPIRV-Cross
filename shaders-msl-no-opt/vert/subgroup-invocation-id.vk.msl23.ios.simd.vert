#version 450
#extension GL_KHR_shader_subgroup_basic : require
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_shuffle : require

layout(location = 0) in vec4 inPos;
layout(location = 0) flat out uint vLane;
layout(location = 1) flat out uint vOwnIndex;
layout(location = 2) flat out uvec4 vEqMask;
layout(location = 3) flat out uvec4 vLtMask;

void main()
{
	gl_Position = inPos;
	vLane = gl_SubgroupInvocationID;
	// Reading this invocation's own lane gives back its own value.
	vOwnIndex = subgroupShuffle(uint(gl_VertexIndex), gl_SubgroupInvocationID);
	vEqMask = gl_SubgroupEqMask;
	vLtMask = gl_SubgroupLtMask;
}
