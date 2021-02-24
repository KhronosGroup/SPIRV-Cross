#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 FragColors [[attribute(2)]];
    float4 gl_Position [[attribute(1)]];
};

struct main0_patchIn
{
    float4 FragColor [[attribute(0)]];
    float2 gl_TessLevelInner [[attribute(3)]];
    float4 gl_TessLevelOuter [[attribute(4)]];
    patch_control_point<main0_in> gl_in;
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], uint gl_PrimitiveID [[patch_id]])
{
    main0_out out = {};
    out.gl_Position = (((((float4(1.0) + patchIn.FragColor) + patchIn.gl_in[0].FragColors) + patchIn.gl_in[1].FragColors) + float4(patchIn.gl_TessLevelInner.x)) + float4(patchIn.gl_TessLevelOuter[int(gl_PrimitiveID) & 1])) + patchIn.gl_in[0].gl_Position;
    return out;
}

