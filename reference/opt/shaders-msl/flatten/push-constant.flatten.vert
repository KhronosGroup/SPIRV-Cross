#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

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

struct PushMe
{
    float4x4 MVP;
    float2x2 Rot;
    spvUnsafeArray<float, 4> Arr;
};

struct main0_out
{
    float2 vRot [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float2 Rot [[attribute(0)]];
    float4 Pos [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant PushMe& registers [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = registers.MVP * in.Pos;
    out.vRot = (registers.Rot * in.Rot) + float2(registers.Arr[2]);
    return out;
}

