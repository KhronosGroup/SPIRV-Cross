#version 450
// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
layout(vertices = 2) out;
layout(set = 0, binding = 0, std430) buffer Results { float values[]; } result;
void seed()
{
    float p = float(gl_PrimitiveID) * 16.0;
    gl_TessLevelOuter = float[4](p + 1.000244140625, p + 2.000244140625, p + 3.000244140625, p + 4.000244140625);
    gl_TessLevelInner = float[2](p + 5.000244140625, p + 6.000244140625);
}
void bump(inout float outer[4], inout float inner[2])
{
    for (int i = 0; i < 4; ++i) outer[i] += 0.25;
    for (int i = 0; i < 2; ++i) inner[i] += 0.25;
}
void record()
{
    float outer[4] = gl_TessLevelOuter;
    float inner[2] = gl_TessLevelInner;
    bump(gl_TessLevelOuter, gl_TessLevelInner);
    for (int i = 0; i < 4; ++i) result.values[gl_PrimitiveID * 12 + i] = outer[i];
    for (int i = 0; i < 2; ++i) result.values[gl_PrimitiveID * 12 + 4 + i] = inner[i];
    for (int i = 0; i < 4; ++i) result.values[gl_PrimitiveID * 12 + 6 + i] = gl_TessLevelOuter[i];
    for (int i = 0; i < 2; ++i) result.values[gl_PrimitiveID * 12 + 10 + i] = gl_TessLevelInner[i];
}
void main()
{
    if (gl_InvocationID == 0) seed();
    barrier();
    gl_TessLevelOuter[gl_InvocationID + 2] += 8.0;
    gl_TessLevelInner[gl_InvocationID] += 8.0;
    barrier();
    if (gl_InvocationID == 0) record();
}
