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

struct Vertices
{
    spvUnsafeArray<float2, 1> uvs;
};

struct main0_out
{
    float2 value [[color(0)]];
};

struct main0_in
{
    float3 gl_BaryCoordNoPerspNV [[barycentric_coord, center_no_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]], const device Vertices& _19 [[buffer(0)]], uint gl_PrimitiveID [[primitive_id]])
{
    main0_out out = {};
    int _23 = 3 * int(gl_PrimitiveID);
    out.value = ((_19.uvs[_23] * in.gl_BaryCoordNoPerspNV.x) + (_19.uvs[_23 + 1] * in.gl_BaryCoordNoPerspNV.y)) + (_19.uvs[_23 + 2] * in.gl_BaryCoordNoPerspNV.z);
    return out;
}

