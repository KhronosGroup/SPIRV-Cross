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

struct main0_out
{
    float4 color [[color(0)]];
};

struct main0_in
{
    float4 spvPerVertex21_1 [[user(locn0_corner0), flat]];
    float4 spvPerVertex21_2 [[user(locn0_corner1), flat]];
    float4 spvPerVertex21_3 [[user(locn0_corner2), flat]];
    float4 interpolated [[user(locn2)]];
    float2 spvPerVertex47_4 [[user(locn3_1_corner0), flat]];
    float2 spvPerVertex47_5 [[user(locn3_1_corner1), flat]];
    float2 spvPerVertex47_6 [[user(locn3_1_corner2), flat]];
    float3 gl_BaryCoordEXT [[barycentric_coord, center_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 3> corner = {};
    spvUnsafeArray<float2, 3> _packed = {};
    corner[0] = in.spvPerVertex21_1;
    corner[1] = in.spvPerVertex21_2;
    corner[2] = in.spvPerVertex21_3;
    _packed[0] = in.spvPerVertex47_4;
    _packed[1] = in.spvPerVertex47_5;
    _packed[2] = in.spvPerVertex47_6;
    out.color = ((((corner[0] * in.gl_BaryCoordEXT.x) + (corner[1] * in.gl_BaryCoordEXT.y)) + (corner[2] * in.gl_BaryCoordEXT.z)) + float4(_packed[1], _packed[2])) + in.interpolated;
    return out;
}

