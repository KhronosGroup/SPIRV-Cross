#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 color [[color(0)]];
};

struct main0_in
{
    uint2 scalar_i64_spv_i64_0 [[user(locn0)]];
    uint2 scalar_u64_spv_i64_0 [[user(locn0_2)]];
    uint4 vector_i64_2_spv_i64_0 [[user(locn1)]];
    uint4 vector_u64_3_spv_i64_0 [[user(locn2)]];
    uint2 vector_u64_3_spv_i64_2 [[user(locn3)]];
    uint4 primitive_i64_4_spv_i64_0 [[user(locn4)]];
    uint4 primitive_i64_4_spv_i64_2 [[user(locn5)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    long scalar_i64 = {};
    ulong scalar_u64 = {};
    long2 vector_i64_2 = {};
    ulong3 vector_u64_3 = {};
    long4 primitive_i64_4 = {};
    scalar_i64 = as_type<long>(in.scalar_i64_spv_i64_0);
    scalar_u64 = as_type<ulong>(in.scalar_u64_spv_i64_0);
    vector_i64_2.xy = as_type<long2>(in.vector_i64_2_spv_i64_0);
    vector_u64_3.xy = as_type<ulong2>(in.vector_u64_3_spv_i64_0);
    vector_u64_3.z = as_type<ulong>(in.vector_u64_3_spv_i64_2);
    primitive_i64_4.xy = as_type<long2>(in.primitive_i64_4_spv_i64_0);
    primitive_i64_4.zw = as_type<long2>(in.primitive_i64_4_spv_i64_2);
    bool _14 = scalar_i64 == (-1l);
    bool _23;
    if (_14)
    {
        _23 = scalar_u64 == 2ul;
    }
    else
    {
        _23 = _14;
    }
    bool _36;
    if (_23)
    {
        _36 = all(vector_i64_2 == long2(-3l, 4l));
    }
    else
    {
        _36 = _23;
    }
    bool _50;
    if (_36)
    {
        _50 = all(vector_u64_3 == ulong3(5ul, 6ul, 7ul));
    }
    else
    {
        _50 = _36;
    }
    bool _65;
    if (_50)
    {
        _65 = all(primitive_i64_4 == long4(-8l, 9l, -10l, 11l));
    }
    else
    {
        _65 = _50;
    }
    out.color = float4(_65);
    return out;
}

