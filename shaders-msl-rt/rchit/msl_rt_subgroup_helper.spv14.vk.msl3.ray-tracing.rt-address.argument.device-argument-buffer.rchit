#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_KHR_shader_subgroup_ballot : require

layout(location = 0) rayPayloadInEXT uint payload;

uint helper()
{
	return subgroupBallot(true).x + gl_SubgroupSize + gl_SubgroupInvocationID;
}

void main()
{
	payload = helper();
}
