#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    uint2 a_0 [[user(locn0)]];
    uint4 b_0 [[user(locn1)]];
    uint4 c_0 [[user(locn2)]];
    uint2 c_1 [[user(locn3)]];
    uint4 d_0 [[user(locn4)]];
    uint4 d_1 [[user(locn5)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0(uint gl_VertexIndex [[vertex_id]])
{
    main0_out out = {};
    long a = {};
    ulong2 b = {};
    long3 c = {};
    ulong4 d = {};
    out.gl_Position = float4(1.0);
    a = long(int(gl_VertexIndex)) - 5l;
    b = ulong2(1099511627776ul, 2ul);
    c = long3(-1l, 2l, -3l);
    d.z = 51539607552ul;
    d.w = 7ul;
    ulong4 _56 = d;
    ulong2 _60 = _56.zw + ulong2(1ul);
    d.x = _60.x;
    d.y = _60.y;
    out.a_0 = as_type<uint2>(a);
    out.b_0 = as_type<uint4>(b);
    out.c_0 = as_type<uint4>(c.xy);
    out.c_1 = as_type<uint2>(c.z);
    out.d_0 = as_type<uint4>(d.xy);
    out.d_1 = as_type<uint4>(d.zw);
    return out;
}

