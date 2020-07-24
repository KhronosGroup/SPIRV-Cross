#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _10
{
    uint4 _m0[1024];
};

struct main0_in
{
    uint4 m_19 [[attribute(0)]];
};

kernel void main0(main0_in in [[stage_in]], device _10& _12 [[buffer(0)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], uint3 spvDispatchBase [[grid_origin]])
{
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    uint gl_VertexIndex = gl_GlobalInvocationID.x + spvDispatchBase.x;
    _12._m0[int(gl_VertexIndex)] = in.m_19;
}

