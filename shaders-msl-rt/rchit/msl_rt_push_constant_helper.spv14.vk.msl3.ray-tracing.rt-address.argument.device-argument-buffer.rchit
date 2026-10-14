#version 460
#extension GL_EXT_ray_tracing : require

layout(push_constant, std430) uniform Registers { uint value; } registers;
layout(location = 0) rayPayloadInEXT float payload;

uint readPushConstant()
{
	return registers.value;
}

void main()
{
	payload = float(readPushConstant());
}
