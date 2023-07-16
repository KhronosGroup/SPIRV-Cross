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
void implicit_texture(const device spvDescriptor<texture2d<float>>* textures, const device spvDescriptor<sampler>* texturesSmplr, thread uint& inputId)
{
    uint _30 = inputId;
    float4 d = textures[_30].value.sample(texturesSmplr[_30].value, float2(0.0), level(0.0));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_ssbo(thread uint& inputId, const device spvDescriptor<const device Ssbo*>* ssbo)
{
    uint _52 = inputId;
    if (ssbo[_52]->val == 2u)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void implicit_ubo(thread uint& inputId, const device spvDescriptor<constant Ubo*>* ubo)
{
    uint _68 = inputId;
    if (ubo[_68]->val == 2u)
    {
        discard_fragment();
    }
}

static inline __attribute__((always_inline))
void explicit_texture(texture2d<float> tex, sampler texSmplr)
{
    float4 d = tex.sample(texSmplr, float2(0.0), level(0.0));
    if (d.w > 0.5)
    {
        discard_fragment();
    }
}

fragment void main0(main0_in in [[stage_in]], const device spvDescriptor<const device Ssbo*>* ssbo [[buffer(2)]], const device spvDescriptor<constant Ubo*>* ubo [[buffer(3)]], const device spvDescriptor<texture2d<float>>* textures [[buffer(0)]], const device spvDescriptor<sampler>* texturesSmplr [[buffer(1)]])
{
    implicit_texture(textures, texturesSmplr, in.inputId);
    implicit_ssbo(in.inputId, ssbo);
    implicit_ubo(in.inputId, ubo);
    uint _89 = in.inputId;
    explicit_texture(textures[_89].value, texturesSmplr[_89].value);
}

