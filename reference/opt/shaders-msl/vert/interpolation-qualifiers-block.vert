#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Output
{
    float2 v0;
    float2 v1;
    float3 v2;
    float4 v3;
    float v4;
    float v5;
    float v6;
};

struct main0_out
{
    float2 Output_v0 [[user(locn0)]];
    float2 Output_v1 [[user(locn1)]];
    float3 Output_v2 [[user(locn2)]];
    float4 Output_v3 [[user(locn3)]];
    float Output_v4 [[user(locn4)]];
    float Output_v5 [[user(locn5)]];
    float Output_v6 [[user(locn6)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.Output_v0 = in.Position.xy;
    out.Output_v1 = in.Position.zw;
    out.Output_v2 = float3(in.Position.x, in.Position.z * in.Position.y, in.Position.x);
    out.Output_v3 = in.Position.xxyy;
    out.Output_v4 = in.Position.w;
    out.Output_v5 = in.Position.y;
    out.Output_v6 = in.Position.x * in.Position.w;
    out.gl_Position = in.Position;
    return out;
}

