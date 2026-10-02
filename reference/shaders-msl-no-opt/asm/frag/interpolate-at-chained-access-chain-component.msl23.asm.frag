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
    spvUnsafeArray<float4, 2> a;
};

struct main0_out
{
    float4 o [[color(0)]];
};

struct main0_in
{
    interpolant<float4, interpolation::perspective> blk_a_0 [[user(locn0)]];
    interpolant<float4, interpolation::perspective> blk_a_1 [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Blk blk = {};
    blk.a[0] = in.blk_a_0.interpolate_at_center();
    blk.a[1] = in.blk_a_1.interpolate_at_center();
    out.o = float4(in.blk_a_1.interpolate_at_centroid().z);
    return out;
}

