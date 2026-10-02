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

struct Blk
{
    float2 xy;
    spvUnsafeArray<float, 2> z;
};

struct main0_out
{
    float2 blk_xy [[user(locn0)]];
    float blk_z_0 [[user(locn0_2)]];
    float blk_z_1 [[user(locn1_2)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    Blk blk = {};
    out.gl_Position = float4(1.0);
    blk.xy = float2(1.0, 2.0);
    blk.z[0] = 3.0;
    blk.z[1] = 4.0;
    out.blk_xy = blk.xy;
    out.blk_z_0 = blk.z[0];
    out.blk_z_1 = blk.z[1];
    return out;
}

