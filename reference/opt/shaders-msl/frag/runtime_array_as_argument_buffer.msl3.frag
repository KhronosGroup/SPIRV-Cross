#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

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

template<typename T>
struct spvDescriptor
{
    T value;
    const device T& operator -> () const device
    {
        return value;
    }
    const device T& operator * () const device
    {
        return value;
    }
};

struct Ssbo
{
    uint val;
};

struct Ubo
{
    uint val;
};

struct main0_in
{
    uint inputId [[user(locn0)]];
};

fragment void main0(main0_in in [[stage_in]], const device spvDescriptor<const device Ssbo*>* ssbo [[buffer(4)]], const device spvDescriptor<constant Ubo*>* ubo [[buffer(5)]], const device spvDescriptor<texture2d<float>>* smp_textures [[buffer(0)]], const device spvDescriptor<texture2d<float>>* textures [[buffer(2)]], const device spvDescriptor<texture2d<float>>* images [[buffer(6)]], const device spvDescriptor<sampler>* smp_texturesSmplr [[buffer(1)]], const device spvDescriptor<sampler>* smp [[buffer(3)]], const device spvDescriptor<raytracing::acceleration_structure<raytracing::instancing>>* tlas [[buffer(7)]])
{
    uint _220 = in.inputId;
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery;
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery_1;
    if (smp_textures[_220].value.sample(smp_texturesSmplr[_220].value, float2(0.0), level(0.0)).w > 0.5)
    {
        discard_fragment();
    }
    uint _238 = in.inputId + 8u;
    if (textures[_220].value.sample(smp[_238].value, float2(0.0), level(0.0)).w > 0.5)
    {
        discard_fragment();
    }
    if (ssbo[_220]->val == 2u)
    {
        discard_fragment();
    }
    if (ubo[_220]->val == 2u)
    {
        discard_fragment();
    }
    if (images[_220].value.read(uint2(int2(0))).w > 0.5)
    {
        discard_fragment();
    }
    rayQuery.reset(ray(float3(0.0), float3(1.0), 0.00999999977648258209228515625, 1.0), tlas[in.inputId].value, spvMakeIntersectionParams(0u));
    bool _284 = rayQuery.next();
    if (smp_textures[_220].value.sample(smp_texturesSmplr[_220].value, float2(0.0), level(0.0)).w > 0.5)
    {
        discard_fragment();
    }
    if (textures[_220].value.sample(smp[_220].value, float2(0.0), level(0.0)).w > 0.5)
    {
        discard_fragment();
    }
    if (images[_220].value.read(uint2(int2(0))).w > 0.5)
    {
        discard_fragment();
    }
    rayQuery_1.reset(ray(float3(0.0), float3(1.0), 0.00999999977648258209228515625, 1.0), tlas[in.inputId].value, spvMakeIntersectionParams(0u));
    bool _319 = rayQuery_1.next();
}

