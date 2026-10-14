#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

// Implementation of signed integer mod accurate to SPIR-V specification
template<typename Tx, typename Ty>
inline Tx spvSMod(Tx x, Ty y)
{
    Tx remainder = x - y * (x / y);
    return select(Tx(remainder + y), remainder, remainder == 0 || (x >= 0) == (y >= 0));
}

struct main0_out
{
    float4 outputColor [[color(0)]];
};

struct main0_in
{
    vertex_value<float3> spvPerVertex18_0 [[user(locn0)]];
    float2 uv [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    spvUnsafeArray<float3, 3> color = {};
    color[0] = in.spvPerVertex18_0.get(vertex_index::first);
    color[1] = in.spvPerVertex18_0.get(vertex_index::second);
    color[2] = in.spvPerVertex18_0.get(vertex_index::third);
    out.outputColor = float4(((color[spvSMod(int(gl_FragCoord.x), 3)] + (color[0] * 0.20000000298023223876953125)) + (color[1] * 0.300000011920928955078125)) + (color[2] * 0.5), in.uv.x);
    return out;
}

