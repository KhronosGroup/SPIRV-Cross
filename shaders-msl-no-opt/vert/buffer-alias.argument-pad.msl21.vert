#version 450
#extension GL_EXT_shader_16bit_storage : require

layout(binding = 2) buffer Buf16
{
	uint16_t u16s[];
};

layout(binding = 2) buffer Buf32
{
	uint u32s[];
};

layout(binding = 2) buffer Float32
{
	float f32s[];
};

layout(location = 0) flat out uint u16o;
layout(location = 1) flat out uint u32o;
layout(location = 2) flat out float f32o;

void main()
{
	u16o = uint(u16s[0]);
	u32o = u32s[2];
	f32o = f32s[3];

	gl_Position = vec4(0);
}
