#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 v0;
    float4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[2];
};

static inline __attribute__((always_inline))
void write_in_func(device float4& v0, thread float4& v1, device float4& gl_Position, device float& gl_PointSize, device float (&gl_ClipDistance)[2])
{
    v0 = float4(1.0);
    v1 = float4(2.0);
    gl_Position = float4(3.0);
    gl_PointSize = 4.0;
    gl_ClipDistance[0] = 1.0;
    gl_ClipDistance[1] = 0.5;
}

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], device main0_out* spvOut [[buffer(28)]])
{
    float4 v1 = {};
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    write_in_func(out.v0, v1, out.gl_Position, out.gl_PointSize, out.gl_ClipDistance);
}
