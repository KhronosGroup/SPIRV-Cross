#version 450
layout(location = 0) out VOut
{
	vec4 a;
	layout(location = 3) vec4 b;
	vec4 c;
	layout(location = 1) vec4 d;
	vec4 e;
	layout(location = 6) vec4 f[2];
	vec4 g;
} vout;
void main()
{
	gl_Position = vec4(1.0);
	vout.a = vec4(1.0); vout.b = vec4(2.0); vout.c = vec4(3.0); vout.d = vec4(4.0); vout.e = vec4(5.0);
	vout.f[0] = vec4(6.0); vout.f[1] = vec4(7.0); vout.g = vec4(8.0);
}
