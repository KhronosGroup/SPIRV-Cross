#version 450
// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
#ifdef QUADS
layout(quads, equal_spacing, cw) in;
#elif defined(FRACTIONAL)
layout(triangles, fractional_even_spacing, cw) in;
#else
layout(triangles, equal_spacing, cw) in;
#endif
#ifndef EMPTY
#ifdef RESOURCE
layout(set = 0, binding = 0, std430) readonly buffer Resource { float bias; } data;
#endif
layout(location = 0) in vec4 control[];
layout(location = 1) patch in vec4 patchValue;
#ifdef BLOCK
layout(location = 0) out OutputBlock
{
    vec4 value;
    flat int patchID;
    vec3 domain;
    vec3 factors;
} result;
#define value result.value
#define patchID result.patchID
#define domain result.domain
#define factors result.factors
#else
layout(location = 0) out vec4 value;
layout(location = 1) flat out int patchID;
layout(location = 2) out vec3 domain;
layout(location = 3) out vec3 factors;
#endif
void evaluate(out vec4 result)
{
    result = control[0] * gl_TessCoord.x + control[1] * gl_TessCoord.y + control[2] * gl_TessCoord.z + patchValue;
}
void main()
{
#ifdef CONSTANT
    value = vec4(2.0);
#else
    evaluate(value);
#endif
#ifdef RESOURCE
    value.x += data.bias;
#endif
    gl_Position = value;
#ifndef CONSTANT
    patchID = gl_PrimitiveID;
    domain = gl_TessCoord;
    factors = vec3(gl_TessLevelInner[0], gl_TessLevelOuter[0], gl_PatchVerticesIn);
#endif
}

#else
void main() {}
#endif
