#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

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
    float3 gl_BaryCoordNV [[barycentric_coord, center_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]], const device Vertices& _19 [[buffer(0)]], uint gl_PrimitiveID [[primitive_id]])
{
    main0_out out = {};
    int prim = int(gl_PrimitiveID);
    float2 uv0 = _19.uvs[(3 * prim) + 0];
    float2 uv1 = _19.uvs[(3 * prim) + 1];
    float2 uv2 = _19.uvs[(3 * prim) + 2];
    out.value = ((uv0 * in.gl_BaryCoordNV.x) + (uv1 * in.gl_BaryCoordNV.y)) + (uv2 * in.gl_BaryCoordNV.z);
    return out;
}

