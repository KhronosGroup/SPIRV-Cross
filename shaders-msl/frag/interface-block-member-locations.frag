#version 450
layout(location = 0) in VOut
{
	vec4 a;
	layout(location = 3) vec4 b;
	vec4 c;
	layout(location = 1) vec4 d;
	vec4 e;
	layout(location = 6) vec4 f[2];
	vec4 g;
} vin;
layout(location = 0) out vec4 o;
void main() { o = vin.a + vin.b + vin.c + vin.d + vin.e + vin.f[0] + vin.f[1] + vin.g; }
