#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

static half3 spvLoadVertexRG11B10Half(uint value)
{
    ushort3 res = ushort3((value << 4) & 0x7ff0, (value >> 7) & 0x7ff0, (value >> 17) & 0x7fe0);
    return as_type<half3>(res);
}
static float3 spvLoadVertexRGB9E5Float(uint value)
{
    float exponent = exp2(float(value >> 27)) * exp2(float(-(15 + 9)));
    uint3 mantissa = uint3(value & 0x1ff, extract_bits(value, 9, 9), extract_bits(value, 18, 9));
    return float3(mantissa) * exponent;
}
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

struct spvVertexData0
{
    uchar4 a0;
    uchar spvPad4;
    packed_uchar3 a1;
};
static_assert(alignof(spvVertexData0) == 4, "Unexpected alignment");

struct spvVertexData1
{
    ushort spvPad0[4];
    packed_short4 a3;
};
static_assert(alignof(spvVertexData1) == 2, "Unexpected alignment");

struct spvVertexData2
{
    uint a4;
    uint a5;
    uint a6;
};
static_assert(alignof(spvVertexData2) == 4, "Unexpected alignment");

struct spvVertexData3
{
    uchar spvPad0;
    uchar a8;
    ushort spvPad2[15];
    uint a7;
};
static_assert(alignof(spvVertexData3) == 4, "Unexpected alignment");

main0_in spvLoadVertex(const device spvVertexData0& data0, const device spvVertexData1& data1, const device spvVertexData2& data2, const device spvVertexData3& data3)
{
    main0_in out;
    out.a0 = unpack_unorm4x8_to_float(as_type<uint>(data0.a0));
    out.a1 = float4(float3(uchar3(data0.a1)).bgr, 1);
    out.a3 = max(float4(short4(data1.a3)) * (1.f / 32767), -1.f);
    out.a4 = unpack_unorm10a2_to_float(data2.a4);
    out.a5 = float4(spvLoadVertexRGB9E5Float(data2.a5), 1);
    out.a6 = float4(float3(spvLoadVertexRG11B10Half(data2.a6)), 1);
    out.a7 = data3.a7;
    out.a8 = float4(float(data3.a8) * (1.f / 255), 0, 0, 1);
    return out;
}

vertex main0_out main0(device const uchar* spvVertexBuffer0 [[buffer(0)]], device const uchar* spvVertexBuffer1 [[buffer(1)]], device const uchar* spvVertexBuffer2 [[buffer(2)]], device const uchar* spvVertexBuffer3 [[buffer(3)]], uint gl_VertexIndex [[vertex_id]], uint gl_BaseVertex [[base_vertex]], uint gl_InstanceIndex [[instance_id]], uint gl_BaseInstance [[base_instance]], const device uint* spvVertexStrides [[buffer(19)]])
{
    main0_out out = {};
    main0_in in = spvLoadVertex(*reinterpret_cast<device const spvVertexData0*>(spvVertexBuffer0 + spvVertexStrides[0] * gl_InstanceIndex), *reinterpret_cast<device const spvVertexData1*>(spvVertexBuffer1 + spvVertexStrides[1] * gl_VertexIndex), *reinterpret_cast<device const spvVertexData2*>(spvVertexBuffer2 + spvVertexStrides[2] * gl_BaseInstance), *reinterpret_cast<device const spvVertexData3*>(spvVertexBuffer3 + spvVertexStrides[3] * (gl_BaseInstance + (gl_InstanceIndex - gl_BaseInstance) / 4)));
    out.gl_Position = ((((((in.a0 + in.a1) + in.a3) + in.a4) + in.a5) + in.a6) + float4(float(in.a7))) + in.a8;
    return out;
}

