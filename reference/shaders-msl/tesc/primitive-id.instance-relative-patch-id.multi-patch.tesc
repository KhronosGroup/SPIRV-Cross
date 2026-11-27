#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position;
};

struct main0_patchOut
{
    uint patchId;
};

struct main0_in
{
    uint3 m_74;
    ushort2 m_79;
    float4 gl_Position;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 3];
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 3];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 3, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 3;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 3, spvIndirectParams[1] - 1);
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    if (gl_InvocationID == 0)
    {
        patchOut.patchId = uint(int((gl_PrimitiveID % spvIndirectParams[2])));
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(1.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(1.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(1.0);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(1.0);
    }
}

