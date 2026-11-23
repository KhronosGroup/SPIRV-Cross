#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

inline uint4 spvSubgroupBallot(bool value)
{
    return uint4((simd_vote::vote_t)simd_ballot(value), 0, 0, 0);
}

inline uint spvSubgroupInvocationID()
{
    simd_vote::vote_t ballot = (simd_vote::vote_t)simd_ballot(true);
    uint rank = simd_prefix_exclusive_sum(1u);
    for (uint i = 0; i < rank; i++)
        ballot &= ballot - 1;
    return uint(ctz(ballot));
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

template<>
inline ulong spvSubgroupShuffle(ulong value, ushort lane)
{
    return as_type<ulong>(spvSubgroupShuffle(as_type<uint2>(value), lane));
}

template<>
inline ulong2 spvSubgroupShuffle(ulong2 value, ushort lane)
{
    return ulong2(spvSubgroupShuffle(value.x, lane), spvSubgroupShuffle(value.y, lane));
}

inline ulong3 spvSubgroupShuffle(ulong3 value, ushort lane)
{
    return ulong3(spvSubgroupShuffle(value.xy, lane), spvSubgroupShuffle(value.z, lane));
}

inline ulong4 spvSubgroupShuffle(ulong4 value, ushort lane)
{
    return ulong4(spvSubgroupShuffle(value.xy, lane), spvSubgroupShuffle(value.zw, lane));
}

template<uint N>
inline vec<long, N> spvSubgroupShuffle(vec<long, N> value, ushort lane)
{
    return vec<long, N>(spvSubgroupShuffle(vec<ulong, N>(value), lane));
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
    uint4 gl_SubgroupEqMask = uint4(1 << gl_SubgroupInvocationID, uint3(0));
    uint4 gl_SubgroupLtMask = uint4(extract_bits(0xFFFFFFFF, 0, gl_SubgroupInvocationID), uint3(0));
    out.gl_Position = in.inPos;
    out.vLane = gl_SubgroupInvocationID;
    out.vOwnIndex = spvSubgroupShuffle(uint(int(gl_VertexIndex)), gl_SubgroupInvocationID);
    out.vEqMask = gl_SubgroupEqMask;
    out.vLtMask = gl_SubgroupLtMask;
    return out;
}

