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
    float4 o [[color(0)]];
};

struct main0_in
{
    float2 blk_xy [[user(locn0)]];
    float blk_z_0 [[user(locn0_2)]];
    float blk_z_1 [[user(locn1_2)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Blk blk = {};
    blk.xy = in.blk_xy;
    blk.z[0] = in.blk_z_0;
    blk.z[1] = in.blk_z_1;
    out.o = float4(blk.xy, blk.z[0], blk.z[1]);
    return out;
}

