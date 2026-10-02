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

struct VOut
{
    float4 a;
    float4 b;
    float4 c;
    float4 d;
    float4 e;
    spvUnsafeArray<float4, 2> f;
    float4 g;
};

struct main0_out
{
    float4 vout_a [[user(locn0)]];
    float4 vout_d [[user(locn1)]];
    float4 vout_e [[user(locn2)]];
    float4 vout_b [[user(locn3)]];
    float4 vout_c [[user(locn4)]];
    float4 vout_f_0 [[user(locn6)]];
    float4 vout_f_1 [[user(locn7)]];
    float4 vout_g [[user(locn8)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    VOut vout = {};
    out.gl_Position = float4(1.0);
    vout.a = float4(1.0);
    vout.b = float4(2.0);
    vout.c = float4(3.0);
    vout.d = float4(4.0);
    vout.e = float4(5.0);
    vout.f[0] = float4(6.0);
    vout.f[1] = float4(7.0);
    vout.g = float4(8.0);
    out.vout_a = vout.a;
    out.vout_b = vout.b;
    out.vout_c = vout.c;
    out.vout_d = vout.d;
    out.vout_e = vout.e;
    out.vout_f_0 = vout.f[0];
    out.vout_f_1 = vout.f[1];
    out.vout_g = vout.g;
    return out;
}

