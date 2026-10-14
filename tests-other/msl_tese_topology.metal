// Copyright 2026 Jean-Philippe Meunier
// SPDX-License-Identifier: Apache-2.0
#include <metal_stdlib>
using namespace metal;

static void write_corner(device uint* invocations, device uint* replayOccurrences, device uint* primitiveIndices, uint record, float3 coordinate, uint patch, uint multiplier, uint count)
{
    device uint* words = invocations + 4u + 8u * record;
    words[0] = as_type<uint>(coordinate.x);
    words[1] = as_type<uint>(coordinate.y);
    words[2] = as_type<uint>(coordinate.z);
    words[3] = patch;
    const uint outputRecord = record * multiplier % count;
    words[4] = outputRecord;
    words[5] = words[6] = words[7] = 0u;
    replayOccurrences[2u * record] = outputRecord;
    replayOccurrences[2u * record + 1u] = record / 3u;
    primitiveIndices[record] = outputRecord;
}

static void emit_patch(device uint* invocations, device uint* replayOccurrences, device uint* primitiveIndices, uint factor, uint patch, uint first, uint multiplier, uint count)
{
    const float3 a = float3(1, 0, 0), b = float3(0, 1, 0), c = float3(0, 0, 1);
    if (factor == 1u)
    {
        write_corner(invocations, replayOccurrences, primitiveIndices, first, a, patch, multiplier, count);
        write_corner(invocations, replayOccurrences, primitiveIndices, first + 1u, b, patch, multiplier, count);
        write_corner(invocations, replayOccurrences, primitiveIndices, first + 2u, c, patch, multiplier, count);
    }
    else if (factor == 2u)
    {
        const float3 points[7] = {a, float3(.5, .5, 0), b, float3(0, .5, .5), c, float3(.5, 0, .5), float3(1.0 / 3.0)};
        const uint3 triangles[6] = {uint3(6, 0, 1), uint3(6, 1, 2), uint3(6, 2, 3), uint3(6, 3, 4), uint3(6, 4, 5), uint3(6, 5, 0)};
        for (uint primitive = 0; primitive < 6u; primitive++)
            for (uint corner = 0; corner < 3u; corner++)
                write_corner(invocations, replayOccurrences, primitiveIndices, first + 3u * primitive + corner, points[triangles[primitive][corner]], patch, multiplier, count);
    }
    else
    {
        const float t = 2.0 / 9.0;
        const float3 points[12] = {
            a, float3(2.0 / 3.0, 1.0 / 3.0, 0), float3(1.0 / 3.0, 2.0 / 3.0, 0), b,
            float3(0, 2.0 / 3.0, 1.0 / 3.0), float3(0, 1.0 / 3.0, 2.0 / 3.0), c,
            float3(1.0 / 3.0, 0, 2.0 / 3.0), float3(2.0 / 3.0, 0, 1.0 / 3.0),
            float3(1.0 - 2.0 * t, t, t), float3(t, 1.0 - 2.0 * t, t), float3(t, t, 1.0 - 2.0 * t)
        };
        // Thirteen triangles verified against the native Vulkan factor-3 oracle.
        const uint3 triangles[13] = {
            uint3(9, 8, 0), uint3(9, 0, 1), uint3(10, 2, 3), uint3(10, 3, 4),
            uint3(11, 5, 6), uint3(11, 6, 7), uint3(9, 10, 11),
            uint3(9, 1, 10), uint3(10, 1, 2), uint3(10, 4, 11),
            uint3(11, 4, 5), uint3(11, 7, 9), uint3(9, 7, 8)
        };
        for (uint primitive = 0; primitive < 13u; primitive++)
            for (uint corner = 0; corner < 3u; corner++)
                write_corner(invocations, replayOccurrences, primitiveIndices, first + 3u * primitive + corner, points[triangles[primitive][corner]], patch, multiplier, count);
    }
}

// Bounded two-patch fixture: patch 1 precedes patch 0 to expose patch-addressing errors.
kernel void generate_tese_invocations(device const uint* settings [[buffer(0)]], device uint* invocations [[buffer(1)]], device uint* replayOccurrences [[buffer(2)]], device uint* replayDraw [[buffer(3)]], device uint* primitiveIndices [[buffer(4)]], uint invocation [[thread_position_in_grid]])
{
    if (invocation != 0u) return;
    const uint factor0 = settings[0], factor1 = settings[1], multiplier = settings[2];
    if ((factor0 < 1u || factor0 > 3u) || (factor1 < 1u || factor1 > 3u))
    {
        invocations[0] = 0u;
        replayDraw[3] = 0u;
        return;
    }
    const uint firstPatchCount = factor1 == 1u ? 3u : (factor1 == 2u ? 18u : 39u);
    const uint count = firstPatchCount + (factor0 == 1u ? 3u : (factor0 == 2u ? 18u : 39u));
    invocations[0] = count;
    invocations[1] = invocations[2] = invocations[3] = 0u;
    replayDraw[0] = replayDraw[1] = replayDraw[2] = 0u;
    replayDraw[3] = count;
    emit_patch(invocations, replayOccurrences, primitiveIndices, factor1, 1u, 0u, multiplier, count);
    emit_patch(invocations, replayOccurrences, primitiveIndices, factor0, 0u, firstPatchCount, multiplier, count);
}
