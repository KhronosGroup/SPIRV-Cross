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

struct MVPs
{
    spvUnsafeArray<float4x4, 2> MVP;
};

struct main0_out
{
    float4 gl_Position [[position]];
    uint gl_Layer [[render_target_array_index]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant uint* spvViewMask [[buffer(1)]], constant MVPs& _19 [[buffer(0)]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    uint gl_ViewIndex = spvViewMask[0] + gl_InstanceIndex % spvViewMask[1];
    gl_InstanceIndex /= spvViewMask[1];
    out.gl_Position = _19.MVP[int(gl_ViewIndex)] * in.Position;
    out.gl_Layer = gl_ViewIndex - spvViewMask[0];
    return out;
}

