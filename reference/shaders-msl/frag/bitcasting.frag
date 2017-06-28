#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_in
{
    half4 VertGeom [[user(locn0)]];
};

struct main0_out
{
    half4 FragColor0 [[color(0)]];
    half4 FragColor1 [[color(1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<half> TextureBase [[texture(0)]], sampler TextureBaseSmplr [[sampler(0)]], texture2d<half> TextureDetail [[texture(1)]], sampler TextureDetailSmplr [[sampler(1)]])
{
    main0_out out = {};
    half4 texSample0 = TextureBase.sample(TextureBaseSmplr, float2(in.VertGeom.xy));
    half4 texSample1 = TextureDetail.sample(TextureDetailSmplr, float2(in.VertGeom.xy), int2(3, 2));
    short4 iResult0 = as_type<short4>(texSample0);
    short4 iResult1 = as_type<short4>(texSample1);
    out.FragColor0 = as_type<half4>(iResult0) * as_type<half4>(iResult1);
    ushort4 uResult0 = as_type<ushort4>(texSample0);
    ushort4 uResult1 = as_type<ushort4>(texSample1);
    out.FragColor1 = as_type<half4>(uResult0) * as_type<half4>(uResult1);
    return out;
}

