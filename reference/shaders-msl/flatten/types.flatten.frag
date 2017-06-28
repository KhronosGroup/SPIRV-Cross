#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO1
{
    short4 c;
    short4 d;
};

struct UBO2
{
    ushort4 e;
    ushort4 f;
};

struct UBO0
{
    half4 a;
    half4 b;
};

struct main0_out
{
    half4 FragColor [[color(0)]];
};

fragment main0_out main0(constant UBO1& _14 [[buffer(0)]], constant UBO2& _29 [[buffer(1)]], constant UBO0& _41 [[buffer(2)]])
{
    main0_out out = {};
    out.FragColor = ((((half4(_14.c) + half4(_14.d)) + half4(_29.e)) + half4(_29.f)) + _41.a) + _41.b;
    return out;
}

