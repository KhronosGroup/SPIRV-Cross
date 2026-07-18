#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_tracing_position_fetch : require

layout(location = 0) rayPayloadInEXT vec3 hitValue;

vec3 readPosition(int index)
{
	return gl_HitTriangleVertexPositionsEXT[index];
}

void main()
{
	hitValue = readPosition(0) + readPosition(1) + readPosition(2);
}
