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

struct _RESERVED_IDENTIFIER_FIXUP_gl_PerVertex
{
    float4 _RESERVED_IDENTIFIER_FIXUP_gl_Position;
    float _RESERVED_IDENTIFIER_FIXUP_gl_PointSize;
    spvUnsafeArray<float, 1> _RESERVED_IDENTIFIER_FIXUP_gl_ClipDistance;
    spvUnsafeArray<float, 1> _RESERVED_IDENTIFIER_FIXUP_gl_CullDistance;
};

struct main0_out
{
    float4 gl_Position [[position]];
    float gl_CullDistance_0 [[user(cull0)]];
};

struct main0_in
{
    float2 inPos [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    _RESERVED_IDENTIFIER_FIXUP_gl_PerVertex _13 = {};
    out.gl_Position = float4(in.inPos, 0.0, 1.0);
    _13._RESERVED_IDENTIFIER_FIXUP_gl_CullDistance[0] = in.inPos.x;
    out.gl_CullDistance_0 = _13._RESERVED_IDENTIFIER_FIXUP_gl_CullDistance[0];
    return out;
}

