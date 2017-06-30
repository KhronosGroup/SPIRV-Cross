#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    half4 FragColor [[color(0)]];
};

half4 samp(thread const texture2d<float> t, thread const sampler s)
{
    return t.sample(s, float2(0.5));
}

fragment main0_out main0(texture2d<half> uDepth [[texture(0)]], sampler uSampler [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = samp(uDepth, uSampler);
    return out;
}

