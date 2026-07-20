#pragma clang diagnostic ignored "-Wmissing-prototypes"

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

template<typename T, uint N>
inline void spvArrayCopyFromConstantToStack(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromConstantToThreadGroup(threadgroup T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToStack(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToThreadGroup(threadgroup T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToStack(thread T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToThreadGroup(threadgroup T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToDevice(device T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromConstantToDevice(device T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToDevice(device T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToDevice(device T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToStack(thread T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToThreadGroup(threadgroup T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromConstantToStack(thread T (&dst)[N][M], constant T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromConstantToStack(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromConstantToThreadGroup(threadgroup T (&dst)[N][M], constant T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromConstantToThreadGroup(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromStackToStack(thread T (&dst)[N][M], thread const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromStackToStack(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromStackToThreadGroup(threadgroup T (&dst)[N][M], thread const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromStackToThreadGroup(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromThreadGroupToStack(thread T (&dst)[N][M], threadgroup const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromThreadGroupToStack(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromThreadGroupToThreadGroup(threadgroup T (&dst)[N][M], threadgroup const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromThreadGroupToThreadGroup(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromDeviceToDevice(device T (&dst)[N][M], device const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromDeviceToDevice(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromConstantToDevice(device T (&dst)[N][M], constant T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromConstantToDevice(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromStackToDevice(device T (&dst)[N][M], thread const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromStackToDevice(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromThreadGroupToDevice(device T (&dst)[N][M], threadgroup const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromThreadGroupToDevice(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromDeviceToStack(thread T (&dst)[N][M], device const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromDeviceToStack(dst[i], src[i]);
    }
}

template<typename T, uint N, uint M>
inline void spvArrayCopyFromDeviceToThreadGroup(threadgroup T (&dst)[N][M], device const T (&src)[N][M])
{
    for (uint i = 0; i < N; i++)
    {
        spvArrayCopyFromDeviceToThreadGroup(dst[i], src[i]);
    }
}

intersection_params spvMakeIntersectionParams(uint flags)
{
    intersection_params ip;
    if ((flags & 1) != 0)
        ip.force_opacity(forced_opacity::opaque);
    if ((flags & 2) != 0)
        ip.force_opacity(forced_opacity::non_opaque);
    if ((flags & 4) != 0)
        ip.accept_any_intersection(true);
    if ((flags & 16) != 0)
        ip.set_triangle_cull_mode(triangle_cull_mode::back);
    if ((flags & 32) != 0)
        ip.set_triangle_cull_mode(triangle_cull_mode::front);
    if ((flags & 64) != 0)
        ip.set_opacity_cull_mode(opacity_cull_mode::opaque);
    if ((flags & 128) != 0)
        ip.set_opacity_cull_mode(opacity_cull_mode::non_opaque);
    if ((flags & 256) != 0)
        ip.set_geometry_cull_mode(geometry_cull_mode::triangle);
    if ((flags & 512) != 0)
        ip.set_geometry_cull_mode(geometry_cull_mode::bounding_box);
    return ip;
}

struct spvRayDataWithPayload_29
{
    spvRayDataStorage data;
    float4 payload[2][3];
};

[[visible]] void main0(device const uint2* scene_spvRayMetadata [[buffer(1)]], raytracing::acceleration_structure<raytracing::instancing> scene [[buffer(0)]], thread void* spvRayData, thread spvRayTracingContext& spvRayContext, thread uint& spvRayAction, thread spvRayTracingState& spvRayState)
{
    float4 _115 = (reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload)[1u][2u];
    _115.x = _115.x + 1.0;
    float4 _91[3] = { (reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload)[1u][0u], (reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload)[1u][1u], _115 };
    float4 _81[3] = { (reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload)[0u][0u], (reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload)[0u][1u], (reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload)[0u][2u] };
    float4 _68[2][3] = { { _81[0], _81[1], _81[2] }, { _91[0], _91[1], _91[2] } };
    spvArrayCopyFromStackToStack(reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload, _68);
    float nestedPayload[2][3];
    nestedPayload[1][2] = (reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload)[1][2].x;
    {
        const auto spvTraceRayFlags = 0u;
        const auto spvTraceRayCullMask = 255u;
        const auto spvTraceRaySBTOffset = 0u;
        const auto spvTraceRaySBTStride = 0u;
        const auto spvTraceRayMissIndex = 0u;
        const auto spvTraceRayOrigin = float3(0.0);
        const auto spvTraceRayTMin = 0.0;
        const auto spvTraceRayDirection = float3(0.0, 0.0, -1.0);
        const auto spvTraceRayTMax = 1.0;
        device const uint2* spvTraceRayInstanceMetadata = scene_spvRayMetadata;
        struct spvRayDataWithPayload_37
        {
            spvRayDataStorage data;
            float payload[2][3];
        };
        spvRayDataWithPayload_37 spvTraceData = { { reinterpret_cast<ulong>(spvTraceRayInstanceMetadata), spvRayState.hitAddress + spvRayState.hitTableOffset, spvRayState.swizzleAddress, spvRayState.bufferSizeAddress, spvRayState.dynamicOffsetsAddress, spvRayState.accelerationStructureAddressTableAddress, spvRayState.pushConstantsAddress, spvRayState.descriptorSet0, spvRayState.descriptorSet1, spvRayState.descriptorSet2, spvRayState.descriptorSet3, spvRayState.descriptorSet4, spvRayState.descriptorSet5, spvRayState.descriptorSet6, spvRayState.descriptorSet7 }, {} };
        for (uint spvRayPayloadIndex37_0 = 0; spvRayPayloadIndex37_0 < 2; spvRayPayloadIndex37_0++)
        {
            for (uint spvRayPayloadIndex37_1 = 0; spvRayPayloadIndex37_1 < 3; spvRayPayloadIndex37_1++)
            {
                spvTraceData.payload[spvRayPayloadIndex37_0][spvRayPayloadIndex37_1] = nestedPayload[spvRayPayloadIndex37_0][spvRayPayloadIndex37_1];
            }
        }
        spvRayTracingContext spvRayContextSaved = spvRayContext;
        uint spvRayActionSaved = spvRayAction;
        spvRayContext = {};
        spvRayAction = 0;
        spvRayContext.launchId = spvRayState.launchId;
        spvRayContext.launchSize = spvRayState.launchSize;
        spvRayContext.worldRayOrigin = spvTraceRayOrigin;
        spvRayContext.worldRayDirection = spvTraceRayDirection;
        spvRayContext.rayTmin = spvTraceRayTMin;
        spvRayContext.rayTmax = spvTraceRayTMax;
        spvRayContext.traceRayTmax = spvTraceRayTMax;
        spvRayContext.incomingRayFlags = spvTraceRayFlags;
        spvRayContext.cullMask = (spvTraceRayCullMask & 255u);
        intersection_query<instancing, triangle_data> spvRayQuery;
        const auto spvTraceRayScene = scene;
        spvRayQuery.reset(::metal::raytracing::ray(spvTraceRayOrigin, spvTraceRayDirection, spvTraceRayTMin, spvTraceRayTMax), spvTraceRayScene, (spvTraceRayCullMask & 255u), spvMakeIntersectionParams((spvTraceRayFlags | spvRayState.pipelineFlags)));
        while (spvRayQuery.next())
        {
            if (spvRayQuery.get_candidate_intersection_type() == intersection_type::triangle)
            {
                spvRayContext.objectRayOrigin = spvRayQuery.get_candidate_ray_origin();
                spvRayContext.objectRayDirection = spvRayQuery.get_candidate_ray_direction();
                spvRayContext.rayTmax = spvRayQuery.get_candidate_triangle_distance();
                spvRayContext.instanceId = spvRayQuery.get_candidate_instance_id();
                spvRayContext.instanceCustomIndex = reinterpret_cast<device const uint2*>(spvTraceData.data.instanceMetadataAddress)[spvRayContext.instanceId].x;
                spvRayContext.objectToWorld = spvRayQuery.get_candidate_object_to_world_transform();
                spvRayContext.worldToObject = spvRayQuery.get_candidate_world_to_object_transform();
                *reinterpret_cast<thread float2*>(&spvRayContext.hitAttribute) = spvRayQuery.get_candidate_triangle_barycentric_coord();
                spvRayContext.hitKind = spvRayQuery.is_candidate_triangle_front_facing() ? 0xFEu : 0xFFu;
                spvRayContext.geometryIndex = spvRayQuery.get_candidate_geometry_id();
                spvRayContext.primitiveId = spvRayQuery.get_candidate_primitive_id();
                uint spvCandidateRecordIndex = spvRayQuery.get_candidate_geometry_id() * (spvTraceRaySBTStride & 15u) + (spvTraceRaySBTOffset & 15u) + reinterpret_cast<device const uint2*>(spvTraceData.data.instanceMetadataAddress)[spvRayContext.instanceId].y;
                ulong spvCandidateRecordAddress = spvTraceData.data.hitTableAddress + ulong(spvCandidateRecordIndex) * spvRayState.hitStride;
                spvRayContext.shaderRecordIndex = spvCandidateRecordIndex;
                uint spvCandidateAnyHitHandle = spvRayState.hitSize != 0 ? reinterpret_cast<device const uint*>(spvCandidateRecordAddress)[1] : 0;
                spvRayAction = 0;
                if (spvCandidateAnyHitHandle != 0)
                {
                    spvRayState.recursiveFunctions[spvCandidateAnyHitHandle](&spvTraceData, spvRayContext, spvRayAction, spvRayState);
                }
                if (spvRayAction != 1)
                    spvRayQuery.commit_triangle_intersection();
                if (spvRayAction == 2)
                    spvRayQuery.abort();
            }
            else if (spvRayQuery.get_candidate_intersection_type() == intersection_type::bounding_box)
            {
                if (spvRayState.hitSize != 0)
                {
                    spvRayContext.objectRayOrigin = spvRayQuery.get_candidate_ray_origin();
                    spvRayContext.objectRayDirection = spvRayQuery.get_candidate_ray_direction();
                    spvRayContext.rayTmax = spvRayQuery.get_committed_intersection_type() == intersection_type::none ? spvRayContext.traceRayTmax : spvRayQuery.get_committed_distance();
                    spvRayContext.instanceId = spvRayQuery.get_candidate_instance_id();
                    spvRayContext.instanceCustomIndex = reinterpret_cast<device const uint2*>(spvTraceData.data.instanceMetadataAddress)[spvRayContext.instanceId].x;
                    spvRayContext.objectToWorld = spvRayQuery.get_candidate_object_to_world_transform();
                    spvRayContext.worldToObject = spvRayQuery.get_candidate_world_to_object_transform();
                    spvRayContext.geometryIndex = spvRayQuery.get_candidate_geometry_id();
                    spvRayContext.primitiveId = spvRayQuery.get_candidate_primitive_id();
                    spvRayContext.reportAccepted = false;
                    spvRayContext.reportedDistance = spvRayContext.rayTmax;
                    uint spvCandidateRecordIndex = spvRayQuery.get_candidate_geometry_id() * (spvTraceRaySBTStride & 15u) + (spvTraceRaySBTOffset & 15u) + reinterpret_cast<device const uint2*>(spvTraceData.data.instanceMetadataAddress)[spvRayContext.instanceId].y;
                    ulong spvCandidateRecordAddress = spvTraceData.data.hitTableAddress + ulong(spvCandidateRecordIndex) * spvRayState.hitStride;
                    spvRayContext.shaderRecordIndex = spvCandidateRecordIndex;
                    uint spvCandidateHandle = reinterpret_cast<device const uint*>(spvCandidateRecordAddress)[2];
                    spvRayContext.candidateNonOpaque = spvRayQuery.is_candidate_non_opaque_bounding_box();
                    spvRayAction = 0;
                    spvRayState.recursiveIntersections[spvCandidateHandle](&spvTraceData, spvRayContext, spvRayAction, spvRayState);
                    if (spvRayContext.reportAccepted)
                        spvRayQuery.commit_bounding_box_intersection(spvRayContext.reportedDistance);
                    if (spvRayAction == 2)
                        spvRayQuery.abort();
                }
            }
        }
        if (spvRayQuery.get_committed_intersection_type() == intersection_type::none)
        {
            spvRayContext.rayTmax = spvRayContext.traceRayTmax;
            if (spvRayState.missAddress != 0)
            {
                ulong spvRecordAddress = spvRayState.missAddress + ulong((spvTraceRayMissIndex & 65535u)) * spvRayState.missStride;
                spvRayContext.shaderRecordIndex = (spvTraceRayMissIndex & 65535u);
                uint spvMissHandle = *reinterpret_cast<device const uint*>(spvRecordAddress);
                if (spvMissHandle != 0)
                    spvRayState.recursiveFunctions[spvMissHandle](&spvTraceData, spvRayContext, spvRayAction, spvRayState);
            }
        }
        else if ((spvTraceRayFlags & 8u) == 0 && spvRayState.hitSize != 0)
        {
            spvRayContext.objectRayOrigin = spvRayQuery.get_committed_ray_origin();
            spvRayContext.objectRayDirection = spvRayQuery.get_committed_ray_direction();
            spvRayContext.rayTmax = spvRayQuery.get_committed_distance();
            spvRayContext.instanceId = spvRayQuery.get_committed_instance_id();
            spvRayContext.instanceCustomIndex = reinterpret_cast<device const uint2*>(spvTraceData.data.instanceMetadataAddress)[spvRayContext.instanceId].x;
            spvRayContext.objectToWorld = spvRayQuery.get_committed_object_to_world_transform();
            spvRayContext.worldToObject = spvRayQuery.get_committed_world_to_object_transform();
            if (spvRayQuery.get_committed_intersection_type() == intersection_type::triangle)
            {
                *reinterpret_cast<thread float2*>(&spvRayContext.hitAttribute) = spvRayQuery.get_committed_triangle_barycentric_coord();
                spvRayContext.hitKind = spvRayQuery.is_committed_triangle_front_facing() ? 0xFEu : 0xFFu;
            }
            else
            {
                spvRayContext.hitAttribute = spvRayContext.reportedHitAttribute;
                spvRayContext.hitKind = spvRayContext.reportedHitKind;
            }
            spvRayContext.geometryIndex = spvRayQuery.get_committed_geometry_id();
            spvRayContext.primitiveId = spvRayQuery.get_committed_primitive_id();
            uint spvRecordIndex = spvRayQuery.get_committed_geometry_id() * (spvTraceRaySBTStride & 15u) + (spvTraceRaySBTOffset & 15u) + reinterpret_cast<device const uint2*>(spvTraceData.data.instanceMetadataAddress)[spvRayContext.instanceId].y;
            ulong spvRecordAddress = spvTraceData.data.hitTableAddress + ulong(spvRecordIndex) * spvRayState.hitStride;
            spvRayContext.shaderRecordIndex = spvRecordIndex;
            uint spvRecordHandle = *reinterpret_cast<device const uint*>(spvRecordAddress);
            if (spvRecordHandle != 0)
                spvRayState.recursiveFunctions[spvRecordHandle](&spvTraceData, spvRayContext, spvRayAction, spvRayState);
        }
        for (uint spvRayPayloadIndex37_0 = 0; spvRayPayloadIndex37_0 < 2; spvRayPayloadIndex37_0++)
        {
            for (uint spvRayPayloadIndex37_1 = 0; spvRayPayloadIndex37_1 < 3; spvRayPayloadIndex37_1++)
            {
                nestedPayload[spvRayPayloadIndex37_0][spvRayPayloadIndex37_1] = spvTraceData.payload[spvRayPayloadIndex37_0][spvRayPayloadIndex37_1];
            }
        }
        spvRayContext = spvRayContextSaved;
        spvRayAction = spvRayActionSaved;
    }
    (reinterpret_cast<thread spvRayDataWithPayload_29*>(spvRayData)->payload)[1][2].x = nestedPayload[1][2];
}

