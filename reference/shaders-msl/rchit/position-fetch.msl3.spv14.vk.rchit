#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

struct spvRayTrianglePositions
{
    packed_float3 positions[3];
};
struct spvRayHitAttribute
{
    ulong4 data;
};
struct spvRayTracingContext
{
    uint3 launchId;
    uint3 launchSize;
    float3 worldRayOrigin;
    float3 worldRayDirection;
    float3 objectRayOrigin;
    float3 objectRayDirection;
    float rayTmin;
    float rayTmax;
    uint instanceCustomIndex;
    uint instanceId;
    float4x3 objectToWorld;
    float4x3 worldToObject;
    spvRayHitAttribute hitAttribute;
    uint hitKind;
    uint incomingRayFlags;
    uint geometryIndex;
    uint primitiveId;
    uint cullMask;
    float traceRayTmax;
    float reportedDistance;
    spvRayHitAttribute reportedHitAttribute;
    uint reportedHitKind;
    bool reportAccepted;
    bool candidateNonOpaque;
    uint shaderRecordIndex;
    float3 hitTriangleVertexPositions[3];
};
struct spvRayTracingDispatch
{
    ulong missAddress;
    ulong missStride;
    ulong hitAddress;
    ulong hitStride;
    ulong hitSize;
    ulong callableAddress;
    ulong callableStride;
    ulong hitTableOffset;
    ulong raygenAddress;
    ulong swizzleAddress;
    ulong bufferSizeAddress;
    ulong dynamicOffsetsAddress;
    ulong accelerationStructureAddressTableAddress;
    ulong pushConstantsAddress;
    ulong descriptorSet0;
    ulong descriptorSet1;
    ulong descriptorSet2;
    ulong descriptorSet3;
    ulong descriptorSet4;
    ulong descriptorSet5;
    ulong descriptorSet6;
    ulong descriptorSet7;
    uint pipelineFlags;
};
struct spvRayTracingState;
struct spvRayDataStorage
{
    ulong instanceMetadataAddress;
    ulong hitTableAddress;
    ulong swizzleAddress;
    ulong bufferSizeAddress;
    ulong dynamicOffsetsAddress;
    ulong accelerationStructureAddressTableAddress;
    ulong pushConstantsAddress;
    ulong descriptorSet0;
    ulong descriptorSet1;
    ulong descriptorSet2;
    ulong descriptorSet3;
    ulong descriptorSet4;
    ulong descriptorSet5;
    ulong descriptorSet6;
    ulong descriptorSet7;
};
template<typename T>
struct spvRayDataWithPayload
{
    spvRayDataStorage data;
    T payload;
};
using spvRayFunctionTable = visible_function_table<void(thread void*, thread spvRayTracingContext&, thread uint&, thread spvRayTracingState&, thread intersection_query<instancing, triangle_data>&)>;
using spvCallableFunctionTable = visible_function_table<void(thread void*, thread ulong&, thread spvRayTracingState&)>;
struct spvRayTracingState
{
    spvRayFunctionTable functions;
    spvRayFunctionTable intersections;
    spvCallableFunctionTable callables;
    spvRayFunctionTable recursiveFunctions;
    spvRayFunctionTable recursiveIntersections;
    ulong missAddress;
    ulong missStride;
    ulong hitAddress;
    ulong hitStride;
    ulong hitSize;
    ulong callableAddress;
    ulong callableStride;
    ulong hitTableOffset;
    ulong raygenAddress;
    ulong swizzleAddress;
    ulong bufferSizeAddress;
    ulong dynamicOffsetsAddress;
    ulong accelerationStructureAddressTableAddress;
    ulong pushConstantsAddress;
    ulong descriptorSet0;
    ulong descriptorSet1;
    ulong descriptorSet2;
    ulong descriptorSet3;
    ulong descriptorSet4;
    ulong descriptorSet5;
    ulong descriptorSet6;
    ulong descriptorSet7;
    uint pipelineFlags;
    uint3 launchId;
    uint3 launchSize;
};

static inline __attribute__((always_inline))
float3 readPosition(thread const int& index, thread void* spvRayData, thread spvRayTracingContext& spvRayContext, thread uint& spvRayAction, thread spvRayTracingState& spvRayState, thread intersection_query<instancing, triangle_data>& spvRayPipelineQuery)
{
    return spvRayContext.hitTriangleVertexPositions[index];
}

[[visible]] void main0(thread void* spvRayData, thread spvRayTracingContext& spvRayContext, thread uint& spvRayAction, thread spvRayTracingState& spvRayState, thread intersection_query<instancing, triangle_data>& spvRayPipelineQuery)
{
    if (spvRayContext.hitKind == 0xFEu || spvRayContext.hitKind == 0xFFu)
    {
        const device spvRayTrianglePositions* spvRayPrimitiveData = reinterpret_cast<const device spvRayTrianglePositions*>(spvRayPipelineQuery.get_committed_primitive_data());
        spvRayContext.hitTriangleVertexPositions[0] = float3(spvRayPrimitiveData->positions[0]);
        spvRayContext.hitTriangleVertexPositions[1] = float3(spvRayPrimitiveData->positions[1]);
        spvRayContext.hitTriangleVertexPositions[2] = float3(spvRayPrimitiveData->positions[2]);
    }
    int param = 0;
    int param_1 = 1;
    int param_2 = 2;
    reinterpret_cast<thread spvRayDataWithPayload<float3>*>(spvRayData)->payload = (readPosition(param, spvRayData, spvRayContext, spvRayAction, spvRayState, spvRayPipelineQuery) + readPosition(param_1, spvRayData, spvRayContext, spvRayAction, spvRayState, spvRayPipelineQuery)) + readPosition(param_2, spvRayData, spvRayContext, spvRayAction, spvRayState, spvRayPipelineQuery);
}

