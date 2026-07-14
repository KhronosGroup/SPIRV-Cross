#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

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
using spvRayFunctionTable = visible_function_table<void(thread void*, thread spvRayTracingContext&, thread uint&, thread spvRayTracingState&)>;
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

struct Record
{
    float factor;
};

[[visible]] void main0(thread void* spvRayData, thread spvRayTracingContext& spvRayContext, thread uint& spvRayAction, thread spvRayTracingState& spvRayState)
{
    float _24 = dot(((spvRayContext.worldRayOrigin + spvRayContext.worldRayDirection) + spvRayContext.objectRayOrigin) + spvRayContext.objectRayDirection, float3(1.0));
    float builtins = _24;
    float _83 = _24 + (((((((((((spvRayContext.rayTmin + spvRayContext.rayTmax) + float(spvRayContext.instanceCustomIndex)) + float(spvRayContext.instanceId)) + float(int(spvRayContext.primitiveId))) + float(spvRayContext.geometryIndex)) + float(spvRayContext.hitKind)) + float(spvRayContext.incomingRayFlags)) + float(spvRayContext.cullMask)) + spvRayContext.objectToWorld[0].x) + spvRayContext.worldToObject[0].x) + (*reinterpret_cast<const device Record*>(reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->hitTableAddress + ulong(spvRayContext.shaderRecordIndex) * spvRayState.hitStride + 32)).factor);
    builtins = _83;
    reinterpret_cast<thread spvRayDataWithPayload<float4>*>(spvRayData)->payload = float4(*reinterpret_cast<thread const float2*>(&spvRayContext.hitAttribute), _83, 1.0);
}

