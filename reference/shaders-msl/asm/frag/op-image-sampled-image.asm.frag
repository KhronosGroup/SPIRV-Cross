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

struct push_cb
{
    spvUnsafeArray<float4, 1> cb0;
};

struct main0_out
{
    float4 o0 [[color(0)]];
};

fragment main0_out main0(constant push_cb& _19 [[buffer(0)]], texture2d<float> t0 [[texture(0)]], sampler dummy_sampler [[sampler(0)]])
{
    main0_out out = {};
    float4 r0;
    r0 = float4(_19.cb0[0u].z, _19.cb0[0u].w, r0.z, r0.w);
    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(-1, -2)), as_type<int>(r0.w));
    return out;
}

