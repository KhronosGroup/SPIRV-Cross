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

struct main0_out
{
    float4 outputColor [[color(0)]];
};

struct main0_in
{
    vertex_value<float4> spvPerVertex14_0 [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 3> value = {};
    value[0] = in.spvPerVertex14_0.get(vertex_index::first);
    value[1] = in.spvPerVertex14_0.get(vertex_index::second);
    value[2] = in.spvPerVertex14_0.get(vertex_index::second);
    out.outputColor = (value[0] + (value[1] * 2.0)) + (value[2] * 4.0);
    return out;
}

