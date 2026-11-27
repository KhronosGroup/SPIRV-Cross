#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexBlock
{
    float4 position;
    float4 tile;
};

struct main0_out
{
    float4 Out_position;
    float4 Out_tile;
    float4 gl_Position;
};

struct main0_in
{
    float4 pos [[attribute(0)]];
    float4 tile [[attribute(1)]];
};

kernel void main0(main0_in in [[stage_in]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], device main0_out* spvOut [[buffer(28)]])
{
    VertexBlock Out = {};
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    Out.position.x = in.pos.x;
    Out.position.y = in.pos.y;
    Out.position.z = in.pos.z;
    Out.position.w = 1.0;
    Out.tile.x = in.tile.x;
    Out.tile.y = in.tile.y;
    Out.tile.z = in.tile.z;
    Out.tile.w = in.tile.w;
    out.gl_Position = in.pos;
    out.Out_position = Out.position;
    out.Out_tile = Out.tile;
}

