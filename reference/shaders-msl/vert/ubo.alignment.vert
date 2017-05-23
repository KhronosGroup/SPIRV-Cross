#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4x4 mvp;
    float2 targSize;
    char pad2[8];
    packed_float3 color;
    float opacity;
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
    float3 aNormal [[attribute(1)]];
};

struct main0_out
{
    float2 vSize [[user(locn0)]];
    float3 vNormal [[user(locn1)]];
    float3 vColor [[user(locn2)]];
    float4 gl_Position [[position]];
    float gl_PointSize;
};

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _18 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _18.mvp * in.aVertex;
    out.vNormal = in.aNormal;
    out.vColor = float3(_18.color) * _18.opacity;
    out.vSize = _18.targSize * _18.opacity;
    return out;
}

