#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    half4 FragColor [[color(0)]];
};

fragment main0_out main0(float4 gl_FragCoord [[position]], texture2d_ms<half> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    int2 coord = int2(gl_FragCoord.xy);
    out.FragColor = ((uSampler.read(uint2(coord), 0) + uSampler.read(uint2(coord), 1)) + uSampler.read(uint2(coord), 2)) + uSampler.read(uint2(coord), 3);
    return out;
}

