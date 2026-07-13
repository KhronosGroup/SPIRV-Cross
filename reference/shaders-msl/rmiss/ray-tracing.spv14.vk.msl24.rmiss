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
struct spvCallableData
{
    ulong4 data[8];
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
    ulong callableAddress;
    ulong callableStride;
    ulong hitTableOffset;
    ulong raygenAddress;
    ulong swizzleAddress;
    ulong bufferSizeAddress;
    ulong dynamicOffsetsAddress;
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
    ulong4 payload[128];
    ulong swizzleAddress;
    ulong bufferSizeAddress;
    ulong dynamicOffsetsAddress;
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
using spvRayFunctionTable = visible_function_table<void(thread void*, thread spvRayTracingContext&, thread uint&, thread spvRayTracingState&)>;
using spvCallableFunctionTable = visible_function_table<void(thread uint&, thread ulong&, thread spvRayTracingState&)>;
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
    ulong callableAddress;
    ulong callableStride;
    ulong hitTableOffset;
    ulong raygenAddress;
    ulong swizzleAddress;
    ulong bufferSizeAddress;
    ulong dynamicOffsetsAddress;
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

[[visible]] void main_mvkMiss(device const uint2* as_mvkMetadata [[buffer(1)]], raytracing::acceleration_structure<raytracing::instancing> as [[buffer(0)]], thread void* spvRayData, thread spvRayTracingContext& spvRayContext, thread uint& spvRayAction, thread spvRayTracingState& spvRayState)
{
    float3 origin = float3(float(spvRayContext.launchId.x) / float(spvRayContext.launchSize.x), float(spvRayContext.launchId.y) / float(spvRayContext.launchSize.y), 1.0);
    float3 direction = float3(0.0, 0.0, -1.0);
    {
        ulong spvInstanceMetadataAddressSaved = reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress;
        reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress = reinterpret_cast<ulong>(as_mvkMetadata);
        float spvRayPayloadSaved = *reinterpret_cast<thread float*>(&reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->payload);
        *reinterpret_cast<thread float*>(&reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->payload) = (*reinterpret_cast<thread float*>(&reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->payload));
        spvRayTracingContext spvRayContextSaved = spvRayContext;
        uint spvRayActionSaved = spvRayAction;
        spvRayContext = {};
        spvRayAction = 0;
        spvRayContext.launchId = spvRayState.launchId;
        spvRayContext.launchSize = spvRayState.launchSize;
        spvRayContext.worldRayOrigin = origin;
        spvRayContext.worldRayDirection = direction;
        spvRayContext.rayTmin = 0.0;
        spvRayContext.rayTmax = 1000.0;
        spvRayContext.traceRayTmax = 1000.0;
        spvRayContext.incomingRayFlags = 0u;
        spvRayContext.cullMask = (255u & 255u);
        intersection_query<instancing, triangle_data> spvRayQuery;
        spvRayQuery.reset(ray(origin, direction, 0.0, 1000.0), as, (255u & 255u), spvMakeIntersectionParams((0u | spvRayState.pipelineFlags)));
        while (spvRayQuery.next())
        {
            if (spvRayQuery.get_candidate_intersection_type() == intersection_type::triangle)
            {
                spvRayContext.objectRayOrigin = spvRayQuery.get_candidate_ray_origin();
                spvRayContext.objectRayDirection = spvRayQuery.get_candidate_ray_direction();
                spvRayContext.rayTmax = spvRayQuery.get_candidate_triangle_distance();
                spvRayContext.instanceId = spvRayQuery.get_candidate_instance_id();
                spvRayContext.instanceCustomIndex = reinterpret_cast<device const uint2*>(reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress)[spvRayContext.instanceId].x;
                spvRayContext.objectToWorld = spvRayQuery.get_candidate_object_to_world_transform();
                spvRayContext.worldToObject = spvRayQuery.get_candidate_world_to_object_transform();
                *reinterpret_cast<thread float2*>(&spvRayContext.hitAttribute) = spvRayQuery.get_candidate_triangle_barycentric_coord();
                spvRayContext.hitKind = spvRayQuery.is_candidate_triangle_front_facing() ? 0xFEu : 0xFFu;
                spvRayContext.geometryIndex = spvRayQuery.get_candidate_geometry_id();
                spvRayContext.primitiveId = spvRayQuery.get_candidate_primitive_id();
                uint spvCandidateRecordIndex = spvRayQuery.get_candidate_geometry_id() * (1u & 15u) + (0u & 15u) + reinterpret_cast<device const uint2*>(reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress)[spvRayContext.instanceId].y;
                ulong spvCandidateRecordAddress = reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->hitTableAddress + ulong(spvCandidateRecordIndex) * spvRayState.hitStride;
                spvRayContext.shaderRecordIndex = spvCandidateRecordIndex;
                uint spvCandidateAnyHitHandle = spvRayState.hitStride != 0 ? reinterpret_cast<device const uint*>(spvCandidateRecordAddress)[1] : 0;
                spvRayAction = 0;
                if (spvCandidateAnyHitHandle != 0)
                {
                    spvRayState.recursiveFunctions[spvCandidateAnyHitHandle](spvRayData, spvRayContext, spvRayAction, spvRayState);
                }
                if (spvRayAction != 1)
                    spvRayQuery.commit_triangle_intersection();
                if (spvRayAction == 2)
                    spvRayQuery.abort();
            }
            else if (spvRayQuery.get_candidate_intersection_type() == intersection_type::bounding_box)
            {
                if (spvRayState.hitStride != 0)
                {
                    spvRayContext.objectRayOrigin = spvRayQuery.get_candidate_ray_origin();
                    spvRayContext.objectRayDirection = spvRayQuery.get_candidate_ray_direction();
                    spvRayContext.rayTmax = spvRayQuery.get_committed_intersection_type() == intersection_type::none ? spvRayContext.traceRayTmax : spvRayQuery.get_committed_distance();
                    spvRayContext.instanceId = spvRayQuery.get_candidate_instance_id();
                    spvRayContext.instanceCustomIndex = reinterpret_cast<device const uint2*>(reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress)[spvRayContext.instanceId].x;
                    spvRayContext.objectToWorld = spvRayQuery.get_candidate_object_to_world_transform();
                    spvRayContext.worldToObject = spvRayQuery.get_candidate_world_to_object_transform();
                    spvRayContext.geometryIndex = spvRayQuery.get_candidate_geometry_id();
                    spvRayContext.primitiveId = spvRayQuery.get_candidate_primitive_id();
                    spvRayContext.reportAccepted = false;
                    spvRayContext.reportedDistance = spvRayContext.rayTmax;
                    uint spvCandidateRecordIndex = spvRayQuery.get_candidate_geometry_id() * (1u & 15u) + (0u & 15u) + reinterpret_cast<device const uint2*>(reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress)[spvRayContext.instanceId].y;
                    ulong spvCandidateRecordAddress = reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->hitTableAddress + ulong(spvCandidateRecordIndex) * spvRayState.hitStride;
                    spvRayContext.shaderRecordIndex = spvCandidateRecordIndex;
                    uint spvCandidateHandle = reinterpret_cast<device const uint*>(spvCandidateRecordAddress)[2];
                    spvRayContext.candidateNonOpaque = spvRayQuery.is_candidate_non_opaque_bounding_box();
                    spvRayAction = 0;
                    spvRayState.recursiveIntersections[spvCandidateHandle](spvRayData, spvRayContext, spvRayAction, spvRayState);
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
                ulong spvRecordAddress = spvRayState.missAddress + ulong((0u & 65535u)) * spvRayState.missStride;
                spvRayContext.shaderRecordIndex = (0u & 65535u);
                uint spvMissHandle = *reinterpret_cast<device const uint*>(spvRecordAddress);
                if (spvMissHandle != 0)
                    spvRayState.recursiveFunctions[spvMissHandle](spvRayData, spvRayContext, spvRayAction, spvRayState);
            }
        }
        else if ((0u & 8u) == 0 && spvRayState.hitStride != 0)
        {
            spvRayContext.objectRayOrigin = spvRayQuery.get_committed_ray_origin();
            spvRayContext.objectRayDirection = spvRayQuery.get_committed_ray_direction();
            spvRayContext.rayTmax = spvRayQuery.get_committed_distance();
            spvRayContext.instanceId = spvRayQuery.get_committed_instance_id();
            spvRayContext.instanceCustomIndex = reinterpret_cast<device const uint2*>(reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress)[spvRayContext.instanceId].x;
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
            uint spvRecordIndex = spvRayQuery.get_committed_geometry_id() * (1u & 15u) + (0u & 15u) + reinterpret_cast<device const uint2*>(reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress)[spvRayContext.instanceId].y;
            ulong spvRecordAddress = reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->hitTableAddress + ulong(spvRecordIndex) * spvRayState.hitStride;
            spvRayContext.shaderRecordIndex = spvRecordIndex;
            uint spvRecordHandle = *reinterpret_cast<device const uint*>(spvRecordAddress);
            if (spvRecordHandle != 0)
                spvRayState.recursiveFunctions[spvRecordHandle](spvRayData, spvRayContext, spvRayAction, spvRayState);
        }
        (*reinterpret_cast<thread float*>(&reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->payload)) = *reinterpret_cast<thread float*>(&reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->payload);
        *reinterpret_cast<thread float*>(&reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->payload) = spvRayPayloadSaved;
        reinterpret_cast<thread spvRayDataStorage*>(spvRayData)->instanceMetadataAddress = spvInstanceMetadataAddressSaved;
        spvRayContext = spvRayContextSaved;
        spvRayAction = spvRayActionSaved;
    }
}

