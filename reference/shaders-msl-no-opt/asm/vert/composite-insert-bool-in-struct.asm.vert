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

struct _16
{
    float4 _m0;
    short2 _m1;
};

struct _19
{
    spvUnsafeArray<short2, 2> _m0;
};

constant _16 _32 = {};
constant _19 _34 = {};

struct main0_out
{
    uint2 m_4 [[user(locn0)]];
    uint2 m_5 [[user(locn1)]];
    uint2 m_6 [[user(locn2)]];
    uint2 m_7 [[user(locn3)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0(uint gl_VertexIndex [[vertex_id]])
{
    main0_out out = {};
    bool2 _39 = bool2(gl_VertexIndex > 0u, gl_VertexIndex > 1u);
    _16 _40;
    _40._m0 = float4(0.0, 0.0, 0.0, 1.0);
    _40._m1 = short2(_39);
    _16 _45 = _16{ float4(0.0, 0.0, 0.0, 1.0), short2(bool2(false)) };
    _45._m1 = short2(_39);
    _19 _48;
    _48._m0[1] = short2(_39);
    spvUnsafeArray<bool2, 2> _51 = spvUnsafeArray<bool2, 2>({ _39, bool2(false) });
    _19 _52;
    _52._m0 = { short2(_51[0]), short2(_51[1]) };
    out.gl_Position = _40._m0;
    out.m_4 = uint2(_40._m1);
    out.m_5 = uint2(_45._m1);
    out.m_6 = uint2(_48._m0[1]);
    out.m_7 = uint2(_52._m0[0]);
    return out;
}

