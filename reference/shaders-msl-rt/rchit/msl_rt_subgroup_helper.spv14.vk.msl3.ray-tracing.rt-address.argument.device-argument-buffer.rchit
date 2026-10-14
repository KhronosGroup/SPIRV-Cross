#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

inline uint4 spvSubgroupBallot(bool value)
{
    simd_vote vote = simd_ballot(value);
    // simd_ballot() returns a 64-bit integer-like object, but
    // SPIR-V callers expect a uint4. We must convert.
    // FIXME: This won't include higher bits if Apple ever supports
    // 128 lanes in an SIMD-group.
    return uint4(as_type<uint2>((simd_vote::vote_t)vote), 0, 0);
}

static inline __attribute__((always_inline))
uint helper(SPV_RAY_CONTEXT_ARGS(uint))
{
    uint _19 = spvRayState.SubgroupSize;
    uint _20 = spvSubgroupBallot(true).x + _19;
    uint _22 = spvRayState.SubgroupLocalInvocationId;
    uint _23 = _20 + _22;
    uint _25 = _23;
    return _25;
}

[[visible]] void main0(SPV_RAY_CONTEXT_ARGS(uint))
{
    spvRayData<uint>(SPV_RAY_INCOMING_DATA) = helper(SPV_RAY_CONTEXT_NAMES);
}

