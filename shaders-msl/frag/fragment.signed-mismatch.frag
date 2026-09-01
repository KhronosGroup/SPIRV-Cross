#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

layout(location = 0) out ivec4 out0;
layout(location = 1) out i16vec3 out1[2];
layout(location = 3) out uvec2 out3;
layout(location = 4) out uint16_t out4;
layout(location = 5, component = 2) out uvec2 out5;
layout(location = 6) out vec4 out6;

layout(location = 0) flat in u8vec4 vColor;

void set_globals()
{
	out0 = vColor;
	out1[0] = vColor.yyy;
	out1[1] = vColor.wxz;
	out3[0] = vColor.x;
	out3[1] = vColor.w;
	out4 = vColor.z;
	out5 = vColor.zx;
	out6 = vColor;
}

void main()
{
	set_globals();
}
