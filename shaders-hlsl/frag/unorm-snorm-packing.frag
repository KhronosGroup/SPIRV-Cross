#version 450

layout(location = 0) flat in uint SNORM8;
layout(location = 1) flat in uint UNORM8;
layout(location = 2) flat in vec4 FP32;
layout(location = 0) out vec4 FP32Out;
layout(location = 1) out uint UNORM8Out;
layout(location = 2) out uint SNORM8Out;

void main()
{
	FP32Out = unpackUnorm4x8(UNORM8);
	FP32Out = unpackSnorm4x8(SNORM8);
	UNORM8Out = packUnorm4x8(FP32);
	SNORM8Out = packSnorm4x8(FP32);
}
