#version 450

uniform Transform
{
    mat4 transform;
} block;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out VertexOut
{
    vec4 color;
} outputs;

void main()
{
    gl_Position = block.transform*vec4(position, 1.0);
    outputs.color = color;
}
