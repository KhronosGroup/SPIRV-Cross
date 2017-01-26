#version 310 es

layout(std140) uniform UBO
{
    layout(column_major) mat4 M1C;
    layout(row_major) mat4 M1R;
    layout(column_major) mat2x4 M2C;
    layout(row_major) mat2x4 M2R;
};

out vec4 oA;
out vec4 oB;
out vec4 oC;
out vec4 oD;
out vec4 oE;

void main()
{
	gl_Position = vec4(0.0);
	oA = M1C[1];
	oB = M1R[1];
	oC = M2C[1];
	oD = M2R[0];
	oE = vec4(M1C[1][2], M1R[1][2], M2C[1][2], M2R[1][2]);
}
