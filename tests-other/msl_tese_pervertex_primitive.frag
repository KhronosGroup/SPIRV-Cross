#version 450
// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
// After tessellation, a fragment PrimitiveId is the patch index, which the replayed triangle cannot supply.
#extension GL_EXT_fragment_shader_barycentric : require
layout(location = 2) pervertexEXT in vec3 domain[3];
layout(location = 0) out vec4 color;
void main()
{
    color = vec4(domain[0].x, float(gl_PrimitiveID), 0, 1);
}
