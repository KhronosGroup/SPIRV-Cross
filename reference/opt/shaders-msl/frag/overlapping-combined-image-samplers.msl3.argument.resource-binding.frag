#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template <typename T>
static inline depth2d<T> spvDepthCast(texture2d<T> t)
{
    return reinterpret_cast<thread const depth2d<T> &>(t);
}

template <typename T>
static inline depth2d_array<T> spvDepthCast(texture2d_array<T> t)
{
    return reinterpret_cast<thread const depth2d_array<T> &>(t);
}

template <typename T>
static inline depthcube<T> spvDepthCast(texturecube<T> t)
{
    return reinterpret_cast<thread const depthcube<T> &>(t);
}

template <typename T>
static inline depthcube_array<T> spvDepthCast(texturecube_array<T> t)
{
    return reinterpret_cast<thread const depthcube_array<T> &>(t);
}

struct spvDescriptorSetBuffer0
{
    texture2d<float> s0_2d [[id(0)]];
    // Overlapping binding: texture2d<float> s0_2d_shadow [[id(0)]];
    // Overlapping binding: texturecube<float> s0_cube [[id(0)]];
    sampler s0_2dSmplr [[id(1)]];
    // Overlapping binding: sampler s0_2d_shadowSmplr [[id(1)]];
    // Overlapping binding: sampler s0_cubeSmplr [[id(1)]];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]])
{
    main0_out out = {};
    constant auto &s0_2d_shadow = reinterpret_cast<constant texture2d<float> &>(spvDescriptorSet0.s0_2d);
    constant auto &s0_cube = reinterpret_cast<constant texturecube<float> &>(spvDescriptorSet0.s0_2d);
    constant auto &s0_2d_shadowSmplr = spvDescriptorSet0.s0_2dSmplr;
    constant auto &s0_cubeSmplr = spvDescriptorSet0.s0_2dSmplr;
    out.FragColor = (spvDescriptorSet0.s0_2d.sample(spvDescriptorSet0.s0_2dSmplr, in.vUV.xy) + float4(spvDepthCast(s0_2d_shadow).sample_compare(s0_2d_shadowSmplr, in.vUV.xy, in.vUV.z))) + s0_cube.sample(s0_cubeSmplr, in.vUV);
    return out;
}

