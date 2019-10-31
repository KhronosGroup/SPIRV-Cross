#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct vertexUniformBuffer1
{
    float2x4 transform1;
};

struct vertexUniformBuffer2
{
    float2x4 transform2;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0(constant vertexUniformBuffer1& _20 [[buffer(0)]], constant vertexUniformBuffer2& _26 [[buffer(1)]])
{
    main0_out out = {};
    out.gl_Position = float4(float2x2(float2(_20.transform1[0].xy) + float2(_26.transform2[0].xy), float2(_20.transform1[1].xy) + float2(_26.transform2[1].xy)) * float2(-1.0), 0.0, 1.0);
    return out;
}

