#version 450
layout(triangles) in;

vec4 read_tess_levels()
{
	return vec4(
		gl_TessLevelOuter[0],
		gl_TessLevelOuter[1],
		gl_TessLevelOuter[2],
		gl_TessLevelOuter[3]);
}

void main()
{
	gl_Position = read_tess_levels();
}
