#version 310 es

layout(std140) uniform UBO
{
    mat4 uMVP;
    vec4 A1[2];
    vec4 A2[2][3];
    float A3[3];
    vec4 Offset;
};
in vec4 aVertex;

void main()
{
    vec4 offset = A2[1][1] + A1[1] + A3[2];
    gl_Position = uMVP * aVertex + Offset + offset;
}
