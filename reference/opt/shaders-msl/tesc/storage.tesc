#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct StorageBuffer
{
    int foo;
};

kernel void main0(device StorageBuffer& storageBuffer [[buffer(0)]], texture2d<float, access::write> storageImage [[texture(1)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    if (gl_InvocationID < 1)
    {
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(8.8999996185302734375);
    }
    if (gl_InvocationID < 1)
    {
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(6.900000095367431640625);
    }
    if (gl_InvocationID < 1)
    {
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(8.8999996185302734375);
    }
    if (gl_InvocationID < 1)
    {
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(6.900000095367431640625);
    }
    if (gl_InvocationID < 1)
    {
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(3.900000095367431640625);
    }
    if (gl_InvocationID < 1)
    {
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(4.900000095367431640625);
    }
    if (gl_InvocationID < 1)
    {
        storageBuffer.foo = 0;
    }
    if (gl_InvocationID < 1)
    {
        storageImage.write(float4(0.0), uint2(int2(0)));
    }
}

