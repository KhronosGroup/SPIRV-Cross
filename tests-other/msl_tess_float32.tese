#version 450
// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
layout(triangles, equal_spacing, cw) in;
layout(set = 0, binding = 0, std430) buffer Results { float values[]; } result;
layout(location = 0) out vec4 value;
void readFactors()
{
    float outer[4] = gl_TessLevelOuter;
    float inner[2] = gl_TessLevelInner;
    for (int i = 0; i < 4; ++i) result.values[gl_PrimitiveID * 6 + i] = outer[i];
    for (int i = 0; i < 2; ++i) result.values[gl_PrimitiveID * 6 + 4 + i] = inner[i];
}
void main()
{
    readFactors();
    value = vec4(gl_TessLevelOuter[gl_PrimitiveID % 4], gl_TessLevelInner[gl_PrimitiveID % 2], gl_TessCoord.x, 1.0);
    gl_Position = value;
}
