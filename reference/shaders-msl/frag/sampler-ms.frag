#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(float4 gl_FragCoord [[position]], texture2d_ms<float> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    int2 coord = int2(gl_FragCoord.xy);
    out.FragColor = ((uSampler.read(uint2(coord.xy), 0) + uSampler.read(uint2(coord.xy), 1)) + uSampler.read(uint2(coord.xy), 2)) + uSampler.read(uint2(coord.xy), 3);
    return out;
}

