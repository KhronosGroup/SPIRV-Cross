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

struct _RESERVED_IDENTIFIER_FIXUP_21
{
    float4 _RESERVED_IDENTIFIER_FIXUP_m0;
    float4 _RESERVED_IDENTIFIER_FIXUP_m1;
};

struct _RESERVED_IDENTIFIER_FIXUP_24
{
    spvUnsafeArray<_RESERVED_IDENTIFIER_FIXUP_21, 3> _RESERVED_IDENTIFIER_FIXUP_m0;
};

struct main0_out
{
    float4 _RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_0_RESERVED_IDENTIFIER_FIXUP_m0 [[user(locn0)]];
    float4 _RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_0_RESERVED_IDENTIFIER_FIXUP_m1 [[user(locn1)]];
    float4 _RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_1_RESERVED_IDENTIFIER_FIXUP_m0 [[user(locn2)]];
    float4 _RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_1_RESERVED_IDENTIFIER_FIXUP_m1 [[user(locn3)]];
    float4 _RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_2_RESERVED_IDENTIFIER_FIXUP_m0 [[user(locn4)]];
    float4 _RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_2_RESERVED_IDENTIFIER_FIXUP_m1 [[user(locn5)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 _RESERVED_IDENTIFIER_FIXUP_17 [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    _RESERVED_IDENTIFIER_FIXUP_24 _RESERVED_IDENTIFIER_FIXUP_26 = {};
    out.gl_Position = in._RESERVED_IDENTIFIER_FIXUP_17;
    _RESERVED_IDENTIFIER_FIXUP_26._RESERVED_IDENTIFIER_FIXUP_m0[1]._RESERVED_IDENTIFIER_FIXUP_m1 = float4(-4.0, -9.0, 3.0, 7.0);
    out._RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_0_RESERVED_IDENTIFIER_FIXUP_m0 = _RESERVED_IDENTIFIER_FIXUP_26._RESERVED_IDENTIFIER_FIXUP_m0[0]._RESERVED_IDENTIFIER_FIXUP_m0;
    out._RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_0_RESERVED_IDENTIFIER_FIXUP_m1 = _RESERVED_IDENTIFIER_FIXUP_26._RESERVED_IDENTIFIER_FIXUP_m0[0]._RESERVED_IDENTIFIER_FIXUP_m1;
    out._RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_1_RESERVED_IDENTIFIER_FIXUP_m0 = _RESERVED_IDENTIFIER_FIXUP_26._RESERVED_IDENTIFIER_FIXUP_m0[1]._RESERVED_IDENTIFIER_FIXUP_m0;
    out._RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_1_RESERVED_IDENTIFIER_FIXUP_m1 = _RESERVED_IDENTIFIER_FIXUP_26._RESERVED_IDENTIFIER_FIXUP_m0[1]._RESERVED_IDENTIFIER_FIXUP_m1;
    out._RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_2_RESERVED_IDENTIFIER_FIXUP_m0 = _RESERVED_IDENTIFIER_FIXUP_26._RESERVED_IDENTIFIER_FIXUP_m0[2]._RESERVED_IDENTIFIER_FIXUP_m0;
    out._RESERVED_IDENTIFIER_FIXUP_24_RESERVED_IDENTIFIER_FIXUP_m0_2_RESERVED_IDENTIFIER_FIXUP_m1 = _RESERVED_IDENTIFIER_FIXUP_26._RESERVED_IDENTIFIER_FIXUP_m0[2]._RESERVED_IDENTIFIER_FIXUP_m1;
    return out;
}

