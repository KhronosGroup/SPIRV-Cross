#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _RESERVED_IDENTIFIER_FIXUP_35
{
    float2 _RESERVED_IDENTIFIER_FIXUP_m0;
    float4 _RESERVED_IDENTIFIER_FIXUP_m1;
};

struct _RESERVED_IDENTIFIER_FIXUP_36
{
    float2 _RESERVED_IDENTIFIER_FIXUP_m0;
    _RESERVED_IDENTIFIER_FIXUP_35 _RESERVED_IDENTIFIER_FIXUP_m1;
};

struct main0_out
{
    float _RESERVED_IDENTIFIER_FIXUP_80 [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float2 _RESERVED_IDENTIFIER_FIXUP_36_RESERVED_IDENTIFIER_FIXUP_m0 [[attribute(0)]];
    float2 _RESERVED_IDENTIFIER_FIXUP_36_RESERVED_IDENTIFIER_FIXUP_m1_RESERVED_IDENTIFIER_FIXUP_m0 [[attribute(1)]];
    float4 _RESERVED_IDENTIFIER_FIXUP_36_RESERVED_IDENTIFIER_FIXUP_m1_RESERVED_IDENTIFIER_FIXUP_m1 [[attribute(2)]];
};

struct main0_patchIn
{
    patch_control_point<main0_in> gl_in;
};

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    out.gl_Position = float4((gl_TessCoord.xy * 2.0) - float2(1.0), 0.0, 1.0);
    out._RESERVED_IDENTIFIER_FIXUP_80 = ((float(abs(patchIn.gl_in[0]._RESERVED_IDENTIFIER_FIXUP_36_RESERVED_IDENTIFIER_FIXUP_m1_RESERVED_IDENTIFIER_FIXUP_m1.x - (-4.0)) < 0.001000000047497451305389404296875) * float(abs(patchIn.gl_in[0]._RESERVED_IDENTIFIER_FIXUP_36_RESERVED_IDENTIFIER_FIXUP_m1_RESERVED_IDENTIFIER_FIXUP_m1.y - (-9.0)) < 0.001000000047497451305389404296875)) * float(abs(patchIn.gl_in[0]._RESERVED_IDENTIFIER_FIXUP_36_RESERVED_IDENTIFIER_FIXUP_m1_RESERVED_IDENTIFIER_FIXUP_m1.z - 3.0) < 0.001000000047497451305389404296875)) * float(abs(patchIn.gl_in[0]._RESERVED_IDENTIFIER_FIXUP_36_RESERVED_IDENTIFIER_FIXUP_m1_RESERVED_IDENTIFIER_FIXUP_m1.w - 7.0) < 0.001000000047497451305389404296875);
    return out;
}

