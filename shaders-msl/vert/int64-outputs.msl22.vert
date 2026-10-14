#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) flat out int64_t a;
layout(location = 1) flat out u64vec2 b;
layout(location = 2) flat out i64vec3 c;
layout(location = 4) flat out u64vec4 d;

void main()
{
	gl_Position = vec4(1.0);
	a = int64_t(gl_VertexIndex) - 5l;
	b = u64vec2(1ul << 40, 2ul);
	c = i64vec3(-1l, 2l, -3l);
	d.zw = u64vec2(6ul << 33, 7ul);
	d.xy = d.zw + 1ul;
}
