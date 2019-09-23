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

struct Foo
{
    packed_float3 a;
    float b;
};

struct buf
{
    spvUnsafeArray<Foo, 16> results;
    float4 bar;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment main0_out main0(constant buf& _11 [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    int _67 = int(gl_FragCoord.x) % 16;
    out._entryPointOutput = float4(dot(float3(_11.results[_67].a), _11.bar.xyz), _11.results[_67].b, 0.0, 0.0);
    return out;
}

