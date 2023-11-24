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
    float4 gl_Position;
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
static_assert(sizeof(spvVertexData0) == 8, "Unexpected size");

struct spvVertexData1
{
    ushort spvPad0[4];
    packed_short4 a3;
};
static_assert(alignof(spvVertexData1) == 2, "Unexpected alignment");
static_assert(sizeof(spvVertexData1) == 16, "Unexpected size");

struct spvVertexData2
{
    uint a4;
    uint a5;
    uint a6;
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
    out.a0 = unpack_unorm4x8_to_float(as_type<uint>(data0.a0));
    out.a1 = float4(float3(uchar3(data0.a1)).bgr, 1);
    out.a3 = max(float4(short4(data1.a3)) * (1.f / 32767), -1.f);
    out.a4 = unpack_unorm10a2_to_float(data2.a4);
    out.a5 = float4(spvLoadVertexRGB9E5Float(data2.a5), 1);
    out.a6 = float4(float3(spvLoadVertexRG11B10Half(data2.a6)), 1);
    out.a7 = reinterpret_cast<const device uint&>(data3.data[32]);
    out.a8 = float4(float(data3.data[1]) * (1.f / 255), 0, 0, 1);
    return out;
}

kernel void main0(device const spvVertexData0* spvVertexBuffer0 [[buffer(0)]], device const spvVertexData1* spvVertexBuffer1 [[buffer(1)]], device const spvVertexData2* spvVertexBuffer2 [[buffer(2)]], device const spvVertexData3* spvVertexBuffer3 [[buffer(3)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], uint3 spvDispatchBase [[grid_origin]], device main0_out* spvOut [[buffer(28)]], const device ushort* spvIndices [[buffer(21)]])
{
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    uint gl_VertexIndex = spvIndices[gl_GlobalInvocationID.x] + spvDispatchBase.x;
    uint gl_BaseVertex = spvDispatchBase.x;
    uint gl_InstanceIndex = gl_GlobalInvocationID.y + spvDispatchBase.y;
    uint gl_BaseInstance = spvDispatchBase.y;
    main0_in in = spvLoadVertex(spvVertexBuffer0[gl_InstanceIndex],
                                spvVertexBuffer1[gl_VertexIndex],
                                spvVertexBuffer2[gl_BaseInstance],
                                spvVertexBuffer3[gl_BaseInstance + (gl_InstanceIndex - gl_BaseInstance) / 4]);
    out.gl_Position = ((((((in.a0 + in.a1) + in.a3) + in.a4) + in.a5) + in.a6) + float4(float(in.a7))) + in.a8;
}

