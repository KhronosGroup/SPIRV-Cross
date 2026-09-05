#version 450

layout(location = 0) in vec2 inPos;

void main()
{
	gl_Position = vec4(inPos, 0.0, 1.0);
	gl_CullDistance[0] = inPos.x;
}
