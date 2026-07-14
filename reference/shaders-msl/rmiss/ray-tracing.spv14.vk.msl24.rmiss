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

[[visible]] void main0(device const uint2* as_spvRayMetadata [[buffer(1)]], raytracing::acceleration_structure<raytracing::instancing> as [[buffer(0)]], thread void* spvRayData, thread spvRayTracingContext& spvRayContext, thread uint& spvRayAction, thread spvRayTracingState& spvRayState)
{
    float3 origin = float3(float(spvRayContext.launchId.x) / float(spvRayContext.launchSize.x), float(spvRayContext.launchId.y) / float(spvRayContext.launchSize.y), 1.0);
    float3 direction = float3(0.0, 0.0, -1.0);
    float3 _43 = origin;
    float3 _44 = direction;
    {
        const auto spvTraceRayFlags = 0u;
        const auto spvTraceRayCullMask = 255u;
        const auto spvTraceRaySBTOffset = 0u;
        const auto spvTraceRaySBTStride = 1u;
        const auto spvTraceRayMissIndex = 0u;
        const auto spvTraceRayOrigin = _43;
        const auto spvTraceRayTMin = 0.0;
        const auto spvTraceRayDirection = _44;
        const auto spvTraceRayTMax = 1000.0;
        device const uint2* spvTraceRayInstanceMetadata = as_spvRayMetadata;
        spvRayDataWithPayload<float> spvTraceData = { { reinterpret_cast<ulong>(spvTraceRayInstanceMetadata), spvRayState.hitAddress + spvRayState.hitTableOffset, spvRayState.swizzleAddress, spvRayState.bufferSizeAddress, spvRayState.dynamicOffsetsAddress, spvRayState.accelerationStructureAddressTableAddress, spvRayState.pushConstantsAddress, spvRayState.descriptorSet0, spvRayState.descriptorSet1, spvRayState.descriptorSet2, spvRayState.descriptorSet3, spvRayState.descriptorSet4, spvRayState.descriptorSet5, spvRayState.descriptorSet6, spvRayState.descriptorSet7 }, reinterpret_cast<thread spvRayDataWithPayload<float>*>(spvRayData)->payload };
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
        const auto spvTraceRayScene = as;
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
        reinterpret_cast<thread spvRayDataWithPayload<float>*>(spvRayData)->payload = spvTraceData.payload;
        spvRayContext = spvRayContextSaved;
        spvRayAction = spvRayActionSaved;
    }
}

