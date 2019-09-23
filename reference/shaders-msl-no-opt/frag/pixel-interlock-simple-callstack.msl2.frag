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

struct SSBO1
{
    spvUnsafeArray<uint, 1> values1;
};

struct SSBO0
{
    spvUnsafeArray<uint, 1> values0;
};

static inline __attribute__((always_inline))
void callee2(device SSBO1& v_14, thread float4& gl_FragCoord)
{
    int _25 = int(gl_FragCoord.x);
    v_14.values1[_25]++;
}

static inline __attribute__((always_inline))
void callee(device SSBO1& v_14, thread float4& gl_FragCoord, device SSBO0& v_35)
{
    int _38 = int(gl_FragCoord.x);
    v_35.values0[_38]++;
    callee2(v_14, gl_FragCoord);
}

fragment void main0(device SSBO1& v_14 [[buffer(0), raster_order_group(0)]], device SSBO0& v_35 [[buffer(1), raster_order_group(0)]], float4 gl_FragCoord [[position]])
{
    callee(v_14, gl_FragCoord, v_35);
}

