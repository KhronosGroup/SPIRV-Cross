#version 310 es

layout(std140) uniform UBO
{
    vec4 Data[3][5];
};

in ivec2 aIndex;

void main()
{
    gl_Position = Data[aIndex.x][aIndex.y];
}
