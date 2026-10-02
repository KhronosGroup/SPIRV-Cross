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
    float4 o [[color(0)]];
};

struct main0_in
{
    float4 vin_a [[user(locn0)]];
    float4 vin_d [[user(locn1)]];
    float4 vin_e [[user(locn2)]];
    float4 vin_b [[user(locn3)]];
    float4 vin_c [[user(locn4)]];
    float4 vin_f_0 [[user(locn6)]];
    float4 vin_f_1 [[user(locn7)]];
    float4 vin_g [[user(locn8)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    VOut vin = {};
    vin.a = in.vin_a;
    vin.b = in.vin_b;
    vin.c = in.vin_c;
    vin.d = in.vin_d;
    vin.e = in.vin_e;
    vin.f[0] = in.vin_f_0;
    vin.f[1] = in.vin_f_1;
    vin.g = in.vin_g;
    out.o = ((((((vin.a + vin.b) + vin.c) + vin.d) + vin.e) + vin.f[0]) + vin.f[1]) + vin.g;
    return out;
}

