#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 c;
    float4 gl_Position;
    uint gl_Layer;
};

kernel void main0(constant uint* spvViewMask [[buffer(24)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], uint3 spvDispatchBase [[grid_origin]], device main0_out* spvOut [[buffer(28)]])
{
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    uint gl_ViewIndex = spvViewMask[0] + gl_GlobalInvocationID.y % spvViewMask[1];
    uint gl_InstanceIndex = gl_GlobalInvocationID.y / spvViewMask[1] + spvDispatchBase.y;
    uint gl_VertexIndex = gl_GlobalInvocationID.x + spvDispatchBase.x;
    uint gl_BaseInstance = spvDispatchBase.y;
    out.gl_Position = float4(float(int(gl_ViewIndex)), float(int(gl_InstanceIndex)), float(int(gl_VertexIndex)), 1.0);
    out.c = float4(float(int(gl_ViewIndex)));
    out.gl_Layer = gl_ViewIndex - spvViewMask[0];
}

