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

struct VertexOutput
{
    float4 HPosition;
};

struct TestStruct
{
    float3 position;
    float radius;
};

struct TestStruct_1
{
    packed_float3 position;
    float radius;
};

struct CB0
{
    spvUnsafeArray<TestStruct_1, 16> CB0;
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

static inline __attribute__((always_inline))
float4 _main(thread const VertexOutput& IN, constant CB0& v_26)
{
    TestStruct st;
    st.position = float3(v_26.CB0[1].position);
    st.radius = v_26.CB0[1].radius;
    float4 col = float4(st.position, st.radius);
    return col;
}

fragment main0_out main0(constant CB0& v_26 [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    VertexOutput IN;
    IN.HPosition = gl_FragCoord;
    VertexOutput param = IN;
    VertexOutput param_1 = param;
    out._entryPointOutput = _main(param_1, v_26);
    return out;
}

