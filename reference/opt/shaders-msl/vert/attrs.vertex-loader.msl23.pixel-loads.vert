#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 a0 [[attribute(0)]];
    float4 a1 [[attribute(1)]];
    float4 a3 [[attribute(3)]];
    float4 a4 [[attribute(4)]];
    float4 a5 [[attribute(5)]];
    float4 a6 [[attribute(6)]];
    uint a7 [[attribute(7)]];
    float4 a8 [[attribute(8)]];
};

struct alignas(4) spvVertexData0
{
    rgba8unorm<float4> a0;
    uchar spvPad4;
    packed_uchar3 a1;
};
static_assert(alignof(spvVertexData0) == 4, "Unexpected alignment");
static_assert(sizeof(spvVertexData0) == 8, "Unexpected size");

struct spvVertexData1
{
    ushort spvPad0[4];
    rgba16snorm<float4> a3;
};
static_assert(alignof(spvVertexData1) == 2, "Unexpected alignment");
static_assert(sizeof(spvVertexData1) == 16, "Unexpected size");

struct spvVertexData2
{
    rgb10a2<float4> a4;
    rgb9e5<float3> a5;
    rg11b10f<float3> a6;
    uint spvPad12;
};
static_assert(alignof(spvVertexData2) == 4, "Unexpected alignment");
static_assert(sizeof(spvVertexData2) == 16, "Unexpected size");

struct alignas(4) spvVertexData3
{
    uchar data[16];
};
static_assert(alignof(spvVertexData3) == 4, "Unexpected alignment");
static_assert(sizeof(spvVertexData3) == 16, "Unexpected size");

main0_in spvLoadVertex(const device spvVertexData0& data0, const device spvVertexData1& data1, const device spvVertexData2& data2, const device spvVertexData3& data3)
{
    main0_in out;
    out.a0 = data0.a0;
    out.a1 = float4(float3(data0.a1).bgr, 1);
    out.a3 = data1.a3;
    out.a4 = data2.a4;
    out.a5 = float4(data2.a5, 1);
    out.a6 = float4(data2.a6, 1);
    out.a7 = reinterpret_cast<const device uint&>(data3.data[32]);
    out.a8 = float4(reinterpret_cast<const device r8unorm<float>&>(data3.data[1]), 0, 0, 1);
    return out;
}

vertex main0_out main0(device const spvVertexData0* spvVertexBuffer0 [[buffer(0)]], device const spvVertexData1* spvVertexBuffer1 [[buffer(1)]], device const spvVertexData2* spvVertexBuffer2 [[buffer(2)]], device const spvVertexData3* spvVertexBuffer3 [[buffer(3)]], uint gl_VertexIndex [[vertex_id]], uint gl_BaseVertex [[base_vertex]], uint gl_InstanceIndex [[instance_id]], uint gl_BaseInstance [[base_instance]])
{
    main0_out out = {};
    main0_in in = spvLoadVertex(spvVertexBuffer0[gl_InstanceIndex],
                                spvVertexBuffer1[gl_VertexIndex],
                                spvVertexBuffer2[gl_BaseInstance],
                                spvVertexBuffer3[gl_BaseInstance + (gl_InstanceIndex - gl_BaseInstance) / 4]);
    out.gl_Position = ((((((in.a0 + in.a1) + in.a3) + in.a4) + in.a5) + in.a6) + float4(float(in.a7))) + in.a8;
    return out;
}

