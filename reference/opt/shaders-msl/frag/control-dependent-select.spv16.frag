#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> Samp [[texture(0)]], sampler SampSmplr [[sampler(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float4 _19 = Samp.sample(SampSmplr, in.vUV);
    float4 _23 = Samp.sample(SampSmplr, in.vUV);
    out.FragColor = (gl_FragCoord.x < 100.0) ? _19 : _23;
    return out;
}

