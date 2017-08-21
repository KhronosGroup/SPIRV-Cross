#version 450

layout(binding = 1) uniform samplerBuffer uFloatSampler;
layout(binding = 2) uniform isamplerBuffer uIntSampler;
layout(binding = 3) uniform usamplerBuffer uUintSampler;

void main()
{
	gl_Position =
		texelFetch(uFloatSampler, 20) +
		intBitsToFloat(texelFetch(uIntSampler, 40)) +
		uintBitsToFloat(texelFetch(uUintSampler, 60));
}
