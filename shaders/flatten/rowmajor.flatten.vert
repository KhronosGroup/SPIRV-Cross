#version 310 es

layout(std140) uniform UBO
{
    layout(column_major) mat4 uMVPR;
    layout(row_major) mat4 uMVPC;
};

in vec4 aVertex;

void main()
{
	gl_Position = uMVPR * aVertex + uMVPC * aVertex;
}
