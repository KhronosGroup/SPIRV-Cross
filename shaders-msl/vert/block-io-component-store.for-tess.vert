#version 450

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 tile;

layout(location = 0) out VertexBlock
{
	vec4 position;
	vec4 tile;
} Out;

void main()
{
	Out.position.x = pos.x;
	Out.position.y = pos.y;
	Out.position.z = pos.z;
	Out.position.w = 1.0;
	Out.tile.x = tile.x;
	Out.tile.y = tile.y;
	Out.tile.z = tile.z;
	Out.tile.w = tile.w;
	gl_Position = pos;
}
