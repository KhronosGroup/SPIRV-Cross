#version 450
out float gl_ClipDistance[2];
void main()
{
	gl_Position = vec4(1.0);
	gl_ClipDistance[0] = 1.0;
	gl_ClipDistance[1] = 2.0;
}
