#version 450

layout(triangles, fractional_odd_spacing, cw) in;

layout(location = 0) out vec3 outColor;

void main()
{
    gl_Position = vec4(gl_TessCoord, 1.0);
    outColor = gl_TessCoord;
}
