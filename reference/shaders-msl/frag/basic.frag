#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_in
{
    half2 vTex [[user(locn1)]];
    half4 vColor [[user(locn0)]];
};

struct main0_out
{
    half4 FragColor [[color(0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<half> uTex [[texture(0)]], sampler uTexSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = in.vColor * uTex.sample(uTexSmplr, float2(in.vTex));
    return out;
}

