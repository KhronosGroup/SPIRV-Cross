#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

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

#ifndef SPIRV_CROSS_CONSTANT_ID_10
#define SPIRV_CROSS_CONSTANT_ID_10 2
#endif
constant int Value = SPIRV_CROSS_CONSTANT_ID_10;

struct SpecConstArray
{
    spvUnsafeArray<float4, Value> samples;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int Index [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant SpecConstArray& _15 [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = _15.samples[in.Index];
    return out;
}

