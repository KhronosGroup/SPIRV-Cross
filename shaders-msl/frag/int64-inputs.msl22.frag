#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) flat in int64_t a;
layout(location = 1) flat in u64vec2 b;
layout(location = 2) flat in i64vec3 c;
layout(location = 4) flat in u64vec4 d;
layout(location = 0) out vec4 o;

void main()
{
	o = vec4(float(a + c.z), float(b.y), float(c.x + int64_t(d.w)), float(d.y >> 32ul));
}
