#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

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

inline uint spvSubgroupInvocationID()
{
    simd_vote::vote_t ballot = (simd_vote::vote_t)simd_ballot(true);
    uint rank = simd_prefix_exclusive_sum(1u);
    for (uint i = 0; i < rank; i++)
        ballot &= ballot - 1;
    uint2 ballot2 = as_type<uint2>(ballot);
    return uint(ballot2.x ? ctz(ballot2.x) : (ctz(ballot2.y) + 32));
}

template<typename T>
inline T spvSubgroupShuffle(T value, ushort lane)
{
    return simd_shuffle(value, lane);
}

template<>
inline bool spvSubgroupShuffle(bool value, ushort lane)
{
    return !!simd_shuffle((ushort)value, lane);
}

template<uint N>
inline vec<bool, N> spvSubgroupShuffle(vec<bool, N> value, ushort lane)
{
    return (vec<bool, N>)simd_shuffle((vec<ushort, N>)value, lane);
}

struct main0_out
{
    uint vLane [[user(locn0)]];
    uint vOwnIndex [[user(locn1)]];
    uint4 vEqMask [[user(locn2)]];
    uint4 vLtMask [[user(locn3)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 inPos [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], uint gl_VertexIndex [[vertex_id]])
{
    main0_out out = {};
    uint gl_SubgroupInvocationID = spvSubgroupInvocationID();
    uint4 gl_SubgroupEqMask = gl_SubgroupInvocationID >= 32 ? uint4(0, (1 << (gl_SubgroupInvocationID - 32)), uint2(0)) : uint4(1 << gl_SubgroupInvocationID, uint3(0));
    uint4 gl_SubgroupLtMask = uint4(extract_bits(0xFFFFFFFF, 0, min(gl_SubgroupInvocationID, 32u)), extract_bits(0xFFFFFFFF, 0, (uint)max((int)gl_SubgroupInvocationID - 32, 0)), uint2(0));
    out.gl_Position = in.inPos;
    out.vLane = gl_SubgroupInvocationID;
    out.vOwnIndex = spvSubgroupShuffle(uint(int(gl_VertexIndex)), gl_SubgroupInvocationID);
    out.vEqMask = gl_SubgroupEqMask;
    out.vLtMask = gl_SubgroupLtMask;
    return out;
}

