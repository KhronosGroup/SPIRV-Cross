#version 310 es

in vec3 pos;
in mat4 m;

void main()
{
	gl_Position = m * vec4(pos, 1.0);
}
