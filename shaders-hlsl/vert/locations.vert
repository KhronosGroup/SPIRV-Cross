#version 310 es

struct Foo
{
	vec3 a;
	vec3 b;
	vec3 c;
};

layout(location = 2) in vec4 Input2;
layout(location = 4) in vec4 Input4;
in vec4 Input0;

layout(location = 0) out float vLocation0;
layout(location = 1) out float vLocation1;
out float vLocation2[2];
out Foo vLocation4;
out float vLocation7;

void main()
{
	gl_Position = vec4(1.0) + Input2 + Input4 + Input0;
	vLocation0 = 0.0;
	vLocation1 = 1.0;
	vLocation2[0] = 2.0;
	vLocation2[1] = 2.0;
	Foo foo;
	foo.a = vec3(1.0);
	foo.b = vec3(1.0);
	foo.c = vec3(1.0);
	vLocation4 = foo;
	vLocation7 = 7.0;
}
