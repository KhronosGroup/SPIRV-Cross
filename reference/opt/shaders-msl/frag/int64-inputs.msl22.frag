#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 o [[color(0)]];
};

struct main0_in
{
    uint2 a_0 [[user(locn0)]];
    uint4 b_0 [[user(locn1)]];
    uint4 c_0 [[user(locn2)]];
    uint2 c_1 [[user(locn3)]];
    uint4 d_0 [[user(locn4)]];
    uint4 d_1 [[user(locn5)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    long a = {};
    long3 c = {};
    ulong2 b = {};
    ulong4 d = {};
    a = as_type<long>(in.a_0);
    c.xy = as_type<long2>(in.c_0);
    c.z = as_type<long>(in.c_1);
    b = as_type<ulong2>(in.b_0);
    d.xy = as_type<ulong2>(in.d_0);
    d.zw = as_type<ulong2>(in.d_1);
    out.o = float4(float(a + c.z), float(b.y), float(c.x + long(d.w)), float(d.y >> 32ul));
    return out;
}

