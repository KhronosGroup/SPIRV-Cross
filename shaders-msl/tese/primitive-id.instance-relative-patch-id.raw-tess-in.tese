#version 450
layout(triangles) in;
layout(location = 0) flat out uint pid;
void main() { gl_Position = gl_in[0].gl_Position * gl_TessCoord.x; pid = uint(gl_PrimitiveID); }
