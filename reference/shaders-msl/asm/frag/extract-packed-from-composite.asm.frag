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
    float3 a;
    float b;
};

struct Foo_1
{
    packed_float3 a;
    float b;
};

struct buf
{
    spvUnsafeArray<Foo_1, 16> results;
    float4 bar;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

static inline __attribute__((always_inline))
float4 _main(thread const float4& pos, constant buf& v_11)
{
    int _46 = int(pos.x) % 16;
    Foo foo;
    foo.a = float3(v_11.results[_46].a);
    foo.b = v_11.results[_46].b;
    return float4(dot(foo.a, v_11.bar.xyz), foo.b, 0.0, 0.0);
}

fragment main0_out main0(constant buf& v_11 [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float4 pos = gl_FragCoord;
    float4 param = pos;
    out._entryPointOutput = _main(param, v_11);
    return out;
}

