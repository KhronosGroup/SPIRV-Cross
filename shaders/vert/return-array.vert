#version 310 es

vec4[2] test()
{
	return vec4[](vec4(10.0), vec4(20.0));
}

void main()
{
	gl_Position = test()[0];
}
