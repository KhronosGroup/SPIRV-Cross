#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_in
{
    half2 vTexCoord [[user(locn0)]];
};

struct main0_out
{
    half4 FragColor [[color(0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<half> Texture [[texture(0)]], sampler TextureSmplr [[sampler(0)]])
{
    main0_out out = {};
    half f = Texture.sample(TextureSmplr, float2(in.vTexCoord)).x;
    out.FragColor = half4(f * f);
    return out;
}

