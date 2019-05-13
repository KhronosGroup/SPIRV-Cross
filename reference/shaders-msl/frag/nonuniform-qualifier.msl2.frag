#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4 v[64];
};

struct SSBO
{
    float4 v[1];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int vIndex [[user(locn0)]];
    float2 vUV [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant UBO* ubos_0 [[buffer(0)]], constant UBO* ubos_1 [[buffer(1)]], const device SSBO* ssbos_0 [[buffer(2)]], const device SSBO* ssbos_1 [[buffer(3)]], array<texture2d<float>, 8> uSamplers [[texture(0)]], array<texture2d<float>, 8> uCombinedSamplers [[texture(16)]], array<sampler, 8> uSamps [[sampler(8)]], array<sampler, 8> uCombinedSamplersSmplr [[sampler(16)]])
{
    constant UBO* ubos[] =
    {
        ubos_0,
        ubos_1,
    };

    const device SSBO* ssbos[] =
    {
        ssbos_0,
        ssbos_1,
    };

    main0_out out = {};
    int i = in.vIndex;
    int _24 = i + 10;
    out.FragColor = uSamplers[_24].sample(uSamps[i + 40], in.vUV);
    int _49 = i + 10;
    out.FragColor = uCombinedSamplers[_49].sample(uCombinedSamplersSmplr[_49], in.vUV);
    out.FragColor += ubos[(i + 20)]->v[i + 40];
    out.FragColor += ssbos[(i + 50)]->v[i + 60];
    return out;
}

