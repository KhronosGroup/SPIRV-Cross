#version 450
layout(location = 0) in Blk
{
	layout(location = 0, component = 0) vec2 xy;
	layout(location = 0, component = 2) float z[2];
} blk;
layout(location = 0) out vec4 o;
void main() { o = vec4(blk.xy, blk.z[0], blk.z[1]); }
