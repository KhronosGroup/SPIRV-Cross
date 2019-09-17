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

struct TestStruct
{
    packed_float3 position;
    float radius;
};

struct CB0
{
    spvUnsafeArray<TestStruct, 16> CB0;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment main0_out main0(constant CB0& _26 [[buffer(0)]])
{
    main0_out out = {};
    out._entryPointOutput = float4(_26.CB0[1].position[0], _26.CB0[1].position[1], _26.CB0[1].position[2], _26.CB0[1].radius);
    return out;
}

