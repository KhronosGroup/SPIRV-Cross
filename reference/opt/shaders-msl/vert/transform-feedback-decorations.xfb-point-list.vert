#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertOut
{
    float4 vBar;
};

struct spvXfbBuffer1
{
    char _m0_pad[4];
    packed_float4 gl_Position;
};

struct spvXfbBuffer2
{
    char _m0_pad[16];
    float4 vFoo;
};

struct spvXfbBuffer3
{
    float4 vBar;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], device atomic_uint* spvXfbCounter1 [[buffer(17)]], device spvXfbBuffer1* spvXfb1 [[buffer(13)]], device atomic_uint* spvXfbCounter2 [[buffer(18)]], device spvXfbBuffer2* spvXfb2 [[buffer(14)]], device atomic_uint* spvXfbCounter3 [[buffer(19)]], device spvXfbBuffer3* spvXfb3 [[buffer(15)]])
{
    spvXfbBuffer1 spvXfbOutput1 = {};
    spvXfbBuffer2 spvXfbOutput2 = {};
    spvXfbBuffer3 spvXfbOutput3 = {};
    VertOut _25 = {};
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    spvXfbOutput1.gl_Position = float4(1.0);
    spvXfbOutput2.vFoo = float4(3.0);
    _25.vBar = float4(5.0);
    spvXfbOutput3.vBar = _25.vBar;
    uint spvXfbIndex = gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x;
    uint spvInitOffset1 = atomic_load_explicit(spvXfbCounter1, memory_order_relaxed);
    spvXfb1 = reinterpret_cast<device spvXfbBuffer1*>(reinterpret_cast<device char*>(spvXfb1) + spvInitOffset1);
    spvXfb1[spvXfbIndex] = spvXfbOutput1;
    uint spvInitOffset2 = atomic_load_explicit(spvXfbCounter2, memory_order_relaxed);
    spvXfb2 = reinterpret_cast<device spvXfbBuffer2*>(reinterpret_cast<device char*>(spvXfb2) + spvInitOffset2);
    spvXfb2[spvXfbIndex] = spvXfbOutput2;
    uint spvInitOffset3 = atomic_load_explicit(spvXfbCounter3, memory_order_relaxed);
    spvXfb3 = reinterpret_cast<device spvXfbBuffer3*>(reinterpret_cast<device char*>(spvXfb3) + spvInitOffset3);
    spvXfb3[spvXfbIndex] = spvXfbOutput3;
    threadgroup_barrier(mem_flags::mem_device);
    if (all(gl_GlobalInvocationID.xy == 0))
    {
        uint spvWritten = spvStageInputSize.x * spvStageInputSize.y;
        atomic_store_explicit(spvXfbCounter1, spvInitOffset1 + sizeof(*spvXfb1) * spvWritten, memory_order_relaxed);
        atomic_store_explicit(spvXfbCounter2, spvInitOffset2 + sizeof(*spvXfb2) * spvWritten, memory_order_relaxed);
        atomic_store_explicit(spvXfbCounter3, spvInitOffset3 + sizeof(*spvXfb3) * spvWritten, memory_order_relaxed);
    }
}

