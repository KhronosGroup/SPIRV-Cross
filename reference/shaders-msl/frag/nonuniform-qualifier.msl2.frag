#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

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

fragment main0_out main0(main0_in in [[stage_in]], constant UBO* ubos_0 [[buffer(0)]], constant UBO* ubos_1 [[buffer(1)]], const device SSBO* ssbos_0 [[buffer(2)]], const device SSBO* ssbos_1 [[buffer(3)]], array<texture2d<float>, 8> uSamplers [[texture(0)]], array<texture2d<float>, 8> uCombinedSamplers [[texture(8)]], array<sampler, 7> uSamps [[sampler(0)]], array<sampler, 8> uCombinedSamplersSmplr [[sampler(7)]])
{
    spvUnsafeArray<constant UBO*, 2> ubos =
    {
        ubos_0,
        ubos_1,
    };

    spvUnsafeArray<const device SSBO*, 2> ssbos =
    {
        ssbos_0,
        ssbos_1,
    };

    main0_out out = {};
    int i = in.vIndex;
    int _25 = i + 10;
    int _37 = i + 40;
    out.FragColor = uSamplers[_25].sample(uSamps[_37], in.vUV);
    int _53 = i + 10;
    out.FragColor = uCombinedSamplers[_53].sample(uCombinedSamplersSmplr[_53], in.vUV);
    int _69 = i + 20;
    int _73 = i + 40;
    out.FragColor += ubos[(_69)]->v[_73];
    int _87 = i + 50;
    int _91 = i + 60;
    out.FragColor += ssbos[(_87)]->v[_91];
    return out;
}

