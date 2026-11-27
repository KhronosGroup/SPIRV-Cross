#version 450
layout(isolines, equal_spacing) in;

void main()
{
	gl_Position = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x) +
	              vec4(0.0, gl_TessCoord.y, 0.0, 0.0);
}
