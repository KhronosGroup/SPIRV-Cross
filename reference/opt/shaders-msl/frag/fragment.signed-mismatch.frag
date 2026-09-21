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

struct main0_out
{
    uint4 out0 [[color(0)]];
    ushort4 out1_0 [[color(1)]];
    ushort3 out1_1 [[color(2)]];
    int4 out3 [[color(3)]];
    short2 out4 [[color(4)]];
    short4 m_location_5 [[color(5)]];
    float4 out6 [[color(6)]];
};

struct main0_in
{
    uchar4 vColor [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    int4 out0 = {};
    spvUnsafeArray<short3, 2> out1 = {};
    uint2 out3 = {};
    ushort out4 = {};
    uint2 out5 = {};
    out0 = int4(uint4(in.vColor));
    out1[0] = short3(ushort3(in.vColor.yyy));
    out1[1] = short3(ushort3(in.vColor.wxz));
    out3.x = uint(in.vColor.x);
    out3.y = uint(in.vColor.w);
    out4 = ushort(in.vColor.z);
    out5 = uint2(in.vColor.zx);
    out.out6 = float4(in.vColor);
    out.out0 = uint4(out0);
    out.out1_0 = ushort4(out1[0].xyzz);
    out.out1_1 = ushort3(out1[1]);
    out.out3.xy = int2(out3);
    out.out4.x = short(out4);
    out.m_location_5.zw = short2(out5);
    return out;
}

