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

struct UBO
{
    spvUnsafeArray<float, 1> a;
    spvUnsafeArray<float2, 2> b;
};

struct UBOEnhancedLayout
{
    spvUnsafeArray<float, 1> c;
    spvUnsafeArray<float2, 2> d;
    char _m2_pad[9976];
    float e;
};

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    int vIndex [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant UBO& _17 [[buffer(0)]], constant UBOEnhancedLayout& _30 [[buffer(1)]])
{
    main0_out out = {};
    out.FragColor = (_17.a[in.vIndex] + _30.c[in.vIndex]) + _30.e;
    return out;
}

