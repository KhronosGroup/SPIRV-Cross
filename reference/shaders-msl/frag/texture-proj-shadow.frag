#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_in
{
    float4 vClip4 [[user(locn1)]];
    float3 vClip3 [[user(locn0)]];
    float2 vClip2 [[user(locn2)]];
};

struct main0_out
{
    float FragColor [[color(0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], unknown_depth_texture_type<float> uShadow1D [[texture(0)]], sampler uShadow1DSmplr [[sampler(0)]], depth2d<float> uShadow2D [[texture(1)]], sampler uShadow2DSmplr [[sampler(1)]], texture1d<float> uSampler1D [[texture(2)]], sampler uSampler1DSmplr [[sampler(2)]], texture2d<float> uSampler2D [[texture(3)]], sampler uSampler2DSmplr [[sampler(3)]], texture3d<float> uSampler3D [[texture(4)]], sampler uSampler3DSmplr [[sampler(4)]])
{
    main0_out out = {};
    float4 _20 = in.vClip4;
    _20.y = in.vClip4.w;
    out.FragColor = uShadow1D.sample_compare(uShadow1DSmplr, _20.x / _20.y, in.vClip4.z);
    float4 _30 = in.vClip4;
    _30.z = in.vClip4.w;
    out.FragColor = uShadow2D.sample_compare(uShadow2DSmplr, _30.xy / _30.z, in.vClip4.z);
    out.FragColor = uSampler1D.sample(uSampler1DSmplr, in.vClip2.x / in.vClip2.y).x;
    out.FragColor = uSampler2D.sample(uSampler2DSmplr, in.vClip3.xy / in.vClip3.z).x;
    out.FragColor = uSampler3D.sample(uSampler3DSmplr, in.vClip4.xyz / in.vClip4.w).x;
    return out;
}

