#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

struct spvRayTrianglePositions
{
    packed_float3 positions[3];
};

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

intersection_params spvMakeIntersectionParams(uint flags)
{
    intersection_params ip;
    if ((flags & 1) != 0)
        ip.force_opacity(forced_opacity::opaque);
    if ((flags & 2) != 0)
        ip.force_opacity(forced_opacity::non_opaque);
    if ((flags & 4) != 0)
        ip.accept_any_intersection(true);
    if ((flags & 16) != 0)
        ip.set_triangle_cull_mode(triangle_cull_mode::back);
    if ((flags & 32) != 0)
        ip.set_triangle_cull_mode(triangle_cull_mode::front);
    if ((flags & 64) != 0)
        ip.set_opacity_cull_mode(opacity_cull_mode::opaque);
    if ((flags & 128) != 0)
        ip.set_opacity_cull_mode(opacity_cull_mode::non_opaque);
    if ((flags & 256) != 0)
        ip.set_geometry_cull_mode(geometry_cull_mode::triangle);
    if ((flags & 512) != 0)
        ip.set_geometry_cull_mode(geometry_cull_mode::bounding_box);
    return ip;
}

struct main0_out
{
    float3 o_color [[color(0)]];
};

fragment main0_out main0(raytracing::acceleration_structure<raytracing::instancing> uAS [[buffer(0)]])
{
    main0_out out = {};
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> query;
    query.reset(::metal::raytracing::ray(float3(0.0), float3(1.0, 0.0, 0.0), 0.00999999977648258209228515625, 1000.0), uAS, (255u & 255u), spvMakeIntersectionParams(2u));
    float3 sum = float3(0.0);
    for (;;)
    {
        bool _32 = query.next();
        if (_32)
        {
            uint _36 = uint(query.get_candidate_intersection_type()) - 1;
            if (_36 == 0u)
            {
                spvUnsafeArray<float3, 3> _45;
                const device spvRayTrianglePositions* spvRayPrimitiveData_45 = reinterpret_cast<const device spvRayTrianglePositions*>(query.get_candidate_primitive_data());
                _45[0] = float3(spvRayPrimitiveData_45->positions[0]);
                _45[1] = float3(spvRayPrimitiveData_45->positions[1]);
                _45[2] = float3(spvRayPrimitiveData_45->positions[2]);
                spvUnsafeArray<float3, 3> candidate = _45;
                sum += ((candidate[0] + candidate[1]) + candidate[2]);
                query.commit_triangle_intersection();
            }
            continue;
        }
        else
        {
            break;
        }
    }
    uint _59 = uint(query.get_committed_intersection_type());
    if (_59 == 1u)
    {
        spvUnsafeArray<float3, 3> _65;
        const device spvRayTrianglePositions* spvRayPrimitiveData_65 = reinterpret_cast<const device spvRayTrianglePositions*>(query.get_committed_primitive_data());
        _65[0] = float3(spvRayPrimitiveData_65->positions[0]);
        _65[1] = float3(spvRayPrimitiveData_65->positions[1]);
        _65[2] = float3(spvRayPrimitiveData_65->positions[2]);
        spvUnsafeArray<float3, 3> committed = _65;
        sum += ((committed[0] + committed[1]) + committed[2]);
    }
    out.o_color = sum;
    return out;
}

