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

struct InstanceData
{
    float4x4 MATRIX_MVP;
    float4 Color;
};

struct gInstanceData
{
    spvUnsafeArray<InstanceData, 1> _data;
};

struct main0_out
{
    float4 _entryPointOutput_Color [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 PosL [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], const device gInstanceData& gInstanceData_1 [[buffer(0)]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    out.gl_Position = float4(in.PosL, 1.0) * gInstanceData_1._data[gl_InstanceIndex].MATRIX_MVP;
    out._entryPointOutput_Color = gInstanceData_1._data[gl_InstanceIndex].Color;
    return out;
}

