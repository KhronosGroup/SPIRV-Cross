#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    uint pid [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 gl_Position [[attribute(0)]];
};

struct main0_patchIn
{
    patch_control_point<main0_in> gl_in;
};

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float3 gl_TessCoord [[position_in_patch]], uint gl_PrimitiveID [[patch_id]], constant uint* spvIndirectParams [[buffer(29)]])
{
    main0_out out = {};
    out.gl_Position = patchIn.gl_in[0].gl_Position * gl_TessCoord.x;
    out.pid = uint(int((gl_PrimitiveID % spvIndirectParams[2])));
    return out;
}

