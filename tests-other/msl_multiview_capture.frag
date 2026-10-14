// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
#version 450
#extension GL_EXT_multiview : require
#extension GL_EXT_fragment_shader_barycentric : require
layout(location = 0) pervertexEXT in vec4 value[3];
layout(location = 1) in vec2 ordinary;
layout(location = 0) out vec4 color;
void main()
{
    color = vec4(ordinary, float(gl_ViewIndex), 1.0);
    color += value[0] * gl_BaryCoordEXT.x + value[1] * gl_BaryCoordEXT.y + value[2] * gl_BaryCoordEXT.z;
    color.xyz += gl_BaryCoordNoPerspEXT;
}
