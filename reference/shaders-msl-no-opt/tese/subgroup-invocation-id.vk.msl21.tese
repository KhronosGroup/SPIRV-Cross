#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

inline uint spvSubgroupInvocationID()
{
    simd_vote::vote_t ballot = (simd_vote::vote_t)simd_ballot(true);
    uint rank = simd_prefix_exclusive_sum(1u);
    for (uint i = 0; i < rank; i++)
        ballot &= ballot - 1;
    uint2 ballot2 = as_type<uint2>(ballot);
    return uint(ballot2.x ? ctz(ballot2.x) : (ctz(ballot2.y) + 32));
}

struct main0_out
{
    uint vLane [[user(locn0)]];
    float4 gl_Position [[position]];
};

[[ patch(triangle, 0) ]] vertex main0_out main0(float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    uint gl_SubgroupInvocationID = spvSubgroupInvocationID();
    out.gl_Position = float4(gl_TessCoord, 1.0);
    out.vLane = gl_SubgroupInvocationID;
    return out;
}

