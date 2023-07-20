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
};

template<typename T>
struct spvBufferDescriptor
{
    T value;
    int length;
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
    uint data[1];
};

struct Ubo
{
    uint val;
};

struct main0_in
{
    uint inputId [[user(locn0)]];
};

static inline __attribute__((always_inline))
void implicit_combined_texture(const device spvDescriptor<texture2d<float>>* smp_textures, const device spvDescriptor<sampler>* smp_texturesSmplr, thread uint& inputId)
{
    uint _56 = inputId;
    float4 d = smp_textures[_56].value.sample(smp_texturesSmplr[_56].value, float2(0.0), level(0.0));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_texture(thread uint& inputId, const device spvDescriptor<texture2d<float>>* textures, const device spvDescriptor<sampler>* smp)
{
    uint _78 = inputId;
    uint _87 = inputId + 8u;
    float4 d = textures[_78].value.sample(smp[_87].value, float2(0.0), level(0.0));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_ssbo(thread uint& inputId, const device spvBufferDescriptor<const device Ssbo*>* ssbo)
{
    uint _104 = inputId;
    if (ssbo[_104]->val == 2u)
    {
        discard_fragment();
    }
    if (int((ssbo[123].length - 4) / 4) == 25)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_ubo(thread uint& inputId, const device spvBufferDescriptor<constant Ubo*>* ubo)
{
    uint _130 = inputId;
    if (ubo[_130]->val == 2u)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_image(thread uint& inputId, const device spvDescriptor<texture2d<float>>* images)
{
    uint _143 = inputId;
    float4 d = images[_143].value.read(uint2(int2(0)));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_tlas(thread uint& inputId, thread raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data>& rayQuery, const device spvDescriptor<raytracing::acceleration_structure<raytracing::instancing>>* tlas)
{
    rayQuery.reset(ray(float3(0.0), float3(1.0), 0.00999999977648258209228515625, 1.0), tlas[inputId].value, spvMakeIntersectionParams(0u));
    bool _171 = rayQuery.next();
}

static inline __attribute__((always_inline))
void explicit_comb_texture(texture2d<float> tex, sampler texSmplr)
{
    float4 d = tex.sample(texSmplr, float2(0.0), level(0.0));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void explicit_texture(texture2d<float> tex, sampler smp)
{
    float4 d = tex.sample(smp, float2(0.0), level(0.0));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void explicit_image(texture2d<float> tex)
{
    float4 d = tex.read(uint2(int2(0)));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void explicit_tlas(const raytracing::acceleration_structure<raytracing::instancing> tlas, thread raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data>& rayQuery_1)
{
    rayQuery_1.reset(ray(float3(0.0), float3(1.0), 0.00999999977648258209228515625, 1.0), tlas, spvMakeIntersectionParams(0u));
    bool _203 = rayQuery_1.next();
}

fragment void main0(main0_in in [[stage_in]], const device spvBufferDescriptor<const device Ssbo*>* ssbo [[buffer(4)]], const device spvBufferDescriptor<constant Ubo*>* ubo [[buffer(5)]], const device spvDescriptor<texture2d<float>>* smp_textures [[buffer(0)]], const device spvDescriptor<texture2d<float>>* textures [[buffer(2)]], const device spvDescriptor<texture2d<float>>* images [[buffer(6)]], const device spvDescriptor<sampler>* smp_texturesSmplr [[buffer(1)]], const device spvDescriptor<sampler>* smp [[buffer(3)]], const device spvDescriptor<raytracing::acceleration_structure<raytracing::instancing>>* tlas [[buffer(7)]])
{
    implicit_combined_texture(smp_textures, smp_texturesSmplr, in.inputId);
    implicit_texture(in.inputId, textures, smp);
    implicit_ssbo(in.inputId, ssbo);
    implicit_ubo(in.inputId, ubo);
    implicit_image(in.inputId, images);
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery;
    implicit_tlas(in.inputId, rayQuery, tlas);
    uint _211 = in.inputId;
    explicit_comb_texture(smp_textures[_211].value, smp_texturesSmplr[_211].value);
    uint _215 = in.inputId;
    uint _217 = in.inputId;
    explicit_texture(textures[_215].value, smp[_217].value);
    uint _222 = in.inputId;
    explicit_image(images[_222].value);
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery_1;
    explicit_tlas(tlas[in.inputId].value, rayQuery_1);
}

