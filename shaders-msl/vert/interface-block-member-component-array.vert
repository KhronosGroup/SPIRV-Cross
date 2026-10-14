#version 450
layout(location = 0) out Blk
{
	layout(location = 0, component = 0) vec2 xy;
	layout(location = 0, component = 2) float z[2];
} blk;
void main() { gl_Position = vec4(1.0); blk.xy = vec2(1.0, 2.0); blk.z[0] = 3.0; blk.z[1] = 4.0; }
