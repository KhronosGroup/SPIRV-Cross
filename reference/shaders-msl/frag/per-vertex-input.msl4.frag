#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_vertex_value>

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
    vertex_value<float4> spvPerVertex0 [[user(locn0)]];
    float3 gl_BaryCoordEXT [[barycentric_coord, center_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 3> vertex_color = {};
    vertex_color[0] = in.spvPerVertex0.get(vertex_index::first);
    vertex_color[1] = in.spvPerVertex0.get(vertex_index::second);
    vertex_color[2] = in.spvPerVertex0.get(vertex_index::third);
    out.color = ((vertex_color[0] * in.gl_BaryCoordEXT.x) + (vertex_color[1] * in.gl_BaryCoordEXT.y)) + (vertex_color[2] * in.gl_BaryCoordEXT.z);
    return out;
}

