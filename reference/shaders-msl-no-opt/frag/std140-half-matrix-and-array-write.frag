#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct half8 { alignas(16) half4 data; half4 padding_for_std140_fix_your_shader; };
using half2x8 = half8[2];
using half3x8 = half8[3];
using half4x8 = half8[4];
struct ushort8 { alignas(16) ushort4 data; ushort4 padding_for_std140_fix_your_shader; };
struct short8 { alignas(16) short4 data; short4 padding_for_std140_fix_your_shader; };

struct Foo
{
    half2x8 c23;
    half3x8 c32;
    half3x8 r23;
    half2x8 r32;
    half8 h1[6];
    half8 h2[6];
    half8 h3[6];
    half8 h4[6];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(device Foo& _20 [[buffer(0)]])
{
    main0_out out = {};
    ((device half*)&_20.c23[1].data)[2u] = half(1.0);
    ((device half*)&_20.c32[2].data)[1u] = half(2.0);
    ((device half*)&_20.r23[2u])[1] = half(3.0);
    ((device half*)&_20.r32[1u])[2] = half(4.0);
    _20.c23[1].data = half3(half(0.0), half(1.0), half(2.0));
    _20.c32[1].data = half2(half(0.0), half(1.0));
    ((device half*)&_20.r23[0])[1] = half3(half(0.0), half(1.0), half(2.0)).x;
    ((device half*)&_20.r23[1])[1] = half3(half(0.0), half(1.0), half(2.0)).y;
    ((device half*)&_20.r23[2])[1] = half3(half(0.0), half(1.0), half(2.0)).z;
    ((device half*)&_20.r32[0])[1] = half2(half(0.0), half(1.0)).x;
    ((device half*)&_20.r32[1])[1] = half2(half(0.0), half(1.0)).y;
    (device half3&)_20.c23[0] = half2x3(half3(half(1.0), half(2.0), half(3.0)), half3(half(4.0), half(5.0), half(6.0)))[0];
    (device half3&)_20.c23[1] = half2x3(half3(half(1.0), half(2.0), half(3.0)), half3(half(4.0), half(5.0), half(6.0)))[1];
    (device half2&)_20.c32[0] = half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[0];
    (device half2&)_20.c32[1] = half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[1];
    (device half2&)_20.c32[2] = half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[2];
    (device half2&)_20.r23[0] = half2(half2x3(half3(half(1.0), half(2.0), half(3.0)), half3(half(4.0), half(5.0), half(6.0)))[0][0], half2x3(half3(half(1.0), half(2.0), half(3.0)), half3(half(4.0), half(5.0), half(6.0)))[1][0]);
    (device half2&)_20.r23[1] = half2(half2x3(half3(half(1.0), half(2.0), half(3.0)), half3(half(4.0), half(5.0), half(6.0)))[0][1], half2x3(half3(half(1.0), half(2.0), half(3.0)), half3(half(4.0), half(5.0), half(6.0)))[1][1]);
    (device half2&)_20.r23[2] = half2(half2x3(half3(half(1.0), half(2.0), half(3.0)), half3(half(4.0), half(5.0), half(6.0)))[0][2], half2x3(half3(half(1.0), half(2.0), half(3.0)), half3(half(4.0), half(5.0), half(6.0)))[1][2]);
    (device half3&)_20.r32[0] = half3(half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[0][0], half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[1][0], half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[2][0]);
    (device half3&)_20.r32[1] = half3(half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[0][1], half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[1][1], half3x2(half2(half(1.0), half(2.0)), half2(half(3.0), half(4.0)), half2(half(5.0), half(6.0)))[2][1]);
    _20.h1[5].data = half(1.0);
    _20.h2[5].data = half2(half(1.0), half(2.0));
    _20.h3[5].data = half3(half(1.0), half(2.0), half(3.0));
    _20.h4[5].data = half4(half(1.0), half(2.0), half(3.0), half(4.0));
    ((device half*)&_20.h2[5].data)[1u] = half(10.0);
    ((device half*)&_20.h3[5].data)[2u] = half(11.0);
    ((device half*)&_20.h4[5].data)[3u] = half(12.0);
    out.FragColor = float4(1.0);
    return out;
}

