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

struct _35
{
    spvUnsafeArray<uint4, 1024> _m0;
};

struct _40
{
    spvUnsafeArray<uint4, 1024> _m0;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 m_17 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], device _35& _37 [[buffer(0)]], constant _40& _42 [[buffer(1)]])
{
    main0_out out = {};
    out.gl_Position = in.m_17;
    for (int _22 = 0; _22 < 1024; _22++)
    {
        _37._m0[_22] = _42._m0[_22];
    }
}

