#version 330

uniform Transform
{
    mat4 transform;
} block;

in vec3 position;
in vec4 color;

out VertexOut
{
    vec4 color;
} outputs;

void main()
{
    gl_Position = block.transform*vec4(position, 1.0);
    outputs.color = color;
}
