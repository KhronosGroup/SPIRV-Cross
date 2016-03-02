#version 310 es
#extension GL_EXT_tessellation_shader : require
layout(vertices = 1) out;

out patch vec3 vFoo;

void main()
{
    gl_TessLevelInner[0] = 8.900000;
    gl_TessLevelInner[1] = 6.900000;
    gl_TessLevelOuter[0] = 8.900000;
    gl_TessLevelOuter[1] = 6.900000;
    gl_TessLevelOuter[2] = 3.900000;
    gl_TessLevelOuter[3] = 4.900000;
    vFoo = vec3(1.000000);
}

