#pragma clang diagnostic ignored "-Wmissing-prototypes"

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

half4 sample_texture(thread const texture2d<float> tex, thread const sampler& texSmplr, thread const float2& uv)
{
    return tex.sample(texSmplr, float2(uv));
}

fragment main0_out main0(main0_in in [[stage_in]], texture2d<half> uTex [[texture(0)]], sampler uTexSmplr [[sampler(0)]])
{
    main0_out out = {};
    float2 param = in.vTex;
    out.FragColor = in.vColor * sample_texture(uTex, uTexSmplr, param);
    return out;
}

