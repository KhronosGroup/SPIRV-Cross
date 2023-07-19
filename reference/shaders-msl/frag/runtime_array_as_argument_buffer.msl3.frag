#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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

static inline __attribute__((always_inline))
void implicit_combined_texture(const device spvDescriptor<texture2d<float>>* smp_textures, const device spvDescriptor<sampler>* smp_texturesSmplr, thread uint& inputId)
{
    uint _48 = inputId;
    float4 d = smp_textures[_48].value.sample(smp_texturesSmplr[_48].value, float2(0.0), level(0.0));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_texture(thread uint& inputId, const device spvDescriptor<texture2d<float>>* textures, const device spvDescriptor<sampler>* smp)
{
    uint _70 = inputId;
    uint _79 = inputId + 8u;
    float4 d = textures[_70].value.sample(smp[_79].value, float2(0.0), level(0.0));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_ssbo(thread uint& inputId, const device spvDescriptor<const device Ssbo*>* ssbo)
{
    uint _95 = inputId;
    if (ssbo[_95]->val == 2u)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_ubo(thread uint& inputId, const device spvDescriptor<constant Ubo*>* ubo)
{
    uint _111 = inputId;
    if (ubo[_111]->val == 2u)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_image(thread uint& inputId, const device spvDescriptor<texture2d<float>>* images)
{
    uint _124 = inputId;
    float4 d = images[_124].value.read(uint2(int2(0)));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
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

fragment void main0(main0_in in [[stage_in]], const device spvDescriptor<const device Ssbo*>* ssbo [[buffer(4)]], const device spvDescriptor<constant Ubo*>* ubo [[buffer(5)]], const device spvDescriptor<texture2d<float>>* smp_textures [[buffer(0)]], const device spvDescriptor<texture2d<float>>* textures [[buffer(2)]], const device spvDescriptor<texture2d<float>>* images [[buffer(6)]], const device spvDescriptor<sampler>* smp_texturesSmplr [[buffer(1)]], const device spvDescriptor<sampler>* smp [[buffer(3)]])
{
    implicit_combined_texture(smp_textures, smp_texturesSmplr, in.inputId);
    implicit_texture(in.inputId, textures, smp);
    implicit_ssbo(in.inputId, ssbo);
    implicit_ubo(in.inputId, ubo);
    implicit_image(in.inputId, images);
    uint _171 = in.inputId;
    explicit_comb_texture(smp_textures[_171].value, smp_texturesSmplr[_171].value);
    uint _175 = in.inputId;
    uint _177 = in.inputId;
    explicit_texture(textures[_175].value, smp[_177].value);
    uint _182 = in.inputId;
    explicit_image(images[_182].value);
}

