#version 310 es

layout(std140) uniform UBO
{
    uniform mat4 uMVP;
    vec4 A1[2];
    vec4 A2[2][2];
    float A3[3];
    vec4 Offset;
};
in vec4 aVertex;

void main()
{
    gl_Position = uMVP * aVertex + Offset;
}
