#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4x4 uMVP;
};

struct main0_out
{
    float3 vNormal [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex [[attribute(2)]];
    float3 aNormal [[attribute(3)]];
};

struct spvVertexData1
{
    packed_half4 aVertex;
    packed_short4 aNormal;
};
static_assert(alignof(spvVertexData1) == 2, "Unexpected alignment");
static_assert(sizeof(spvVertexData1) == 16, "Unexpected size");

main0_in spvLoadVertex(const device spvVertexData1& data1)
{
    main0_in out;
    out.aVertex = float4(half4(data1.aVertex));
    out.aNormal = max(float4(short4(data1.aNormal)) * (1.f / 32767), -1.f).rgb;
    return out;
}

vertex main0_out main0(device const spvVertexData1* spvVertexBuffer1 [[buffer(1)]], constant UBO& _16 [[buffer(0)]], uint gl_VertexIndex [[vertex_id]], uint gl_BaseVertex [[base_vertex]], uint gl_InstanceIndex [[instance_id]], uint gl_BaseInstance [[base_instance]])
{
    main0_out out = {};
    main0_in in = spvLoadVertex(spvVertexBuffer1[gl_VertexIndex]);
    out.gl_Position = _16.uMVP * in.aVertex;
    out.vNormal = in.aNormal;
    return out;
}

