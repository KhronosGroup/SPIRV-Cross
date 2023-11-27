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
    half2x8 c22;
    half2x8 c22arr[3];
    half2x8 c23;
    half2x8 c24;
    half3x8 c32;
    half3x8 c33;
    half3x8 c34;
    half4x8 c42;
    half4x8 c43;
    half4x8 c44;
    half2x8 r22;
    half2x8 r22arr[3];
    half3x8 r23;
    half4x8 r24;
    half2x8 r32;
    half3x8 r33;
    half4x8 r34;
    half2x8 r42;
    half3x8 r43;
    half4x8 r44;
    half8 h1[6];
    half8 h2[6];
    half8 h3[6];
    half8 h4[6];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant Foo& u [[buffer(0)]])
{
    main0_out out = {};
    half2 c2 = u.c22[0].data.xy + u.c22[1].data.xy;
    c2 = u.c22arr[2][0].data.xy + u.c22arr[2][1].data.xy;
    half3 c3 = u.c23[0].data.xyz + u.c23[1].data.xyz;
    half4 c4 = u.c24[0].data + u.c24[1].data;
    c2 = (u.c32[0].data.xy + u.c32[1].data.xy) + u.c32[2].data.xy;
    c3 = (u.c33[0].data.xyz + u.c33[1].data.xyz) + u.c33[2].data.xyz;
    c4 = (u.c34[0].data + u.c34[1].data) + u.c34[2].data;
    c2 = ((u.c42[0].data.xy + u.c42[1].data.xy) + u.c42[2].data.xy) + u.c42[3].data.xy;
    c3 = ((u.c43[0].data.xyz + u.c43[1].data.xyz) + u.c43[2].data.xyz) + u.c43[3].data.xyz;
    c4 = ((u.c44[0].data + u.c44[1].data) + u.c44[2].data) + u.c44[3].data;
    half c = ((u.c22[0].data.x + u.c22[0].data.y) + u.c22[1].data.x) + u.c22[1].data.y;
    c = ((u.c22arr[2][0].data.x + u.c22arr[2][0].data.y) + u.c22arr[2][1].data.x) + u.c22arr[2][1].data.y;
    half2x2 c22 = half2x2(u.c22[0].data.xy, u.c22[1].data.xy);
    c22 = half2x2(u.c22arr[2][0].data.xy, u.c22arr[2][1].data.xy);
    half2x3 c23 = half2x3(u.c23[0].data.xyz, u.c23[1].data.xyz);
    half2x4 c24 = half2x4(u.c24[0].data, u.c24[1].data);
    half3x2 c32 = half3x2(u.c32[0].data.xy, u.c32[1].data.xy, u.c32[2].data.xy);
    half3x3 c33 = half3x3(u.c33[0].data.xyz, u.c33[1].data.xyz, u.c33[2].data.xyz);
    half3x4 c34 = half3x4(u.c34[0].data, u.c34[1].data, u.c34[2].data);
    half4x2 c42 = half4x2(u.c42[0].data.xy, u.c42[1].data.xy, u.c42[2].data.xy, u.c42[3].data.xy);
    half4x3 c43 = half4x3(u.c43[0].data.xyz, u.c43[1].data.xyz, u.c43[2].data.xyz, u.c43[3].data.xyz);
    half4x4 c44 = half4x4(u.c44[0].data, u.c44[1].data, u.c44[2].data, u.c44[3].data);
    half2 r2 = half2(u.r22[0].data[0], u.r22[1].data[0]) + half2(u.r22[0].data[1], u.r22[1].data[1]);
    r2 = half2(u.r22arr[2][0].data[0], u.r22arr[2][1].data[0]) + half2(u.r22arr[2][0].data[1], u.r22arr[2][1].data[1]);
    half3 r3 = half3(u.r23[0].data[0], u.r23[1].data[0], u.r23[2].data[0]) + half3(u.r23[0].data[1], u.r23[1].data[1], u.r23[2].data[1]);
    half4 r4 = half4(u.r24[0].data[0], u.r24[1].data[0], u.r24[2].data[0], u.r24[3].data[0]) + half4(u.r24[0].data[1], u.r24[1].data[1], u.r24[2].data[1], u.r24[3].data[1]);
    r2 = (half2(u.r32[0].data[0], u.r32[1].data[0]) + half2(u.r32[0].data[1], u.r32[1].data[1])) + half2(u.r32[0].data[2], u.r32[1].data[2]);
    r3 = (half3(u.r33[0].data[0], u.r33[1].data[0], u.r33[2].data[0]) + half3(u.r33[0].data[1], u.r33[1].data[1], u.r33[2].data[1])) + half3(u.r33[0].data[2], u.r33[1].data[2], u.r33[2].data[2]);
    r4 = (half4(u.r34[0].data[0], u.r34[1].data[0], u.r34[2].data[0], u.r34[3].data[0]) + half4(u.r34[0].data[1], u.r34[1].data[1], u.r34[2].data[1], u.r34[3].data[1])) + half4(u.r34[0].data[2], u.r34[1].data[2], u.r34[2].data[2], u.r34[3].data[2]);
    r2 = ((half2(u.r42[0].data[0], u.r42[1].data[0]) + half2(u.r42[0].data[1], u.r42[1].data[1])) + half2(u.r42[0].data[2], u.r42[1].data[2])) + half2(u.r42[0].data[3], u.r42[1].data[3]);
    r3 = ((half3(u.r43[0].data[0], u.r43[1].data[0], u.r43[2].data[0]) + half3(u.r43[0].data[1], u.r43[1].data[1], u.r43[2].data[1])) + half3(u.r43[0].data[2], u.r43[1].data[2], u.r43[2].data[2])) + half3(u.r43[0].data[3], u.r43[1].data[3], u.r43[2].data[3]);
    r4 = ((half4(u.r44[0].data[0], u.r44[1].data[0], u.r44[2].data[0], u.r44[3].data[0]) + half4(u.r44[0].data[1], u.r44[1].data[1], u.r44[2].data[1], u.r44[3].data[1])) + half4(u.r44[0].data[2], u.r44[1].data[2], u.r44[2].data[2], u.r44[3].data[2])) + half4(u.r44[0].data[3], u.r44[1].data[3], u.r44[2].data[3], u.r44[3].data[3]);
    half r = ((u.r22[0u].data[0] + u.r22[1u].data[0]) + u.r22[0u].data[1]) + u.r22[1u].data[1];
    half2x2 r22 = transpose(half2x2(u.r22[0].data.xy, u.r22[1].data.xy));
    half2x3 r23 = transpose(half3x2(u.r23[0].data.xy, u.r23[1].data.xy, u.r23[2].data.xy));
    half2x4 r24 = transpose(half4x2(u.r24[0].data.xy, u.r24[1].data.xy, u.r24[2].data.xy, u.r24[3].data.xy));
    half3x2 r32 = transpose(half2x3(u.r32[0].data.xyz, u.r32[1].data.xyz));
    half3x3 r33 = transpose(half3x3(u.r33[0].data.xyz, u.r33[1].data.xyz, u.r33[2].data.xyz));
    half3x4 r34 = transpose(half4x3(u.r34[0].data.xyz, u.r34[1].data.xyz, u.r34[2].data.xyz, u.r34[3].data.xyz));
    half4x2 r42 = transpose(half2x4(u.r42[0].data, u.r42[1].data));
    half4x3 r43 = transpose(half3x4(u.r43[0].data, u.r43[1].data, u.r43[2].data));
    half4x4 r44 = transpose(half4x4(u.r44[0].data, u.r44[1].data, u.r44[2].data, u.r44[3].data));
    half h1 = u.h1[5].data.x;
    half2 h2 = u.h2[5].data.xy;
    half3 h3 = u.h3[5].data.xyz;
    half4 h4 = half4(u.h4[5].data);
    out.FragColor = float4(1.0);
    return out;
}

