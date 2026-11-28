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

// Implementation of signed integer mod accurate to SPIR-V specification
template<typename Tx, typename Ty>
inline Tx spvSMod(Tx x, Ty y)
{
    Tx remainder = x - y * (x / y);
    return select(Tx(remainder + y), remainder, remainder == 0 || (x >= 0) == (y >= 0));
}

struct Nested
{
    float2 value;
    spvUnsafeArray<float, 2> weights;
};

struct Inputs
{
    float3 color;
    float2x2 basis;
    Nested nested;
};

struct Located
{
    float2 a;
    float b;
};

struct main0_out
{
    float4 outputColor [[color(0)]];
};

struct main0_in
{
    vertex_value<float2> spvPerVertex76_7 [[user(locn0)]];
    vertex_value<float2> spvPerVertex76_8 [[user(locn1)]];
    vertex_value<float2> spvPerVertex84_9 [[user(locn2)]];
    vertex_value<float2> spvPerVertex84_10 [[user(locn3)]];
    vertex_value<float3> spvPerVertex24_0 [[user(locn4)]];
    vertex_value<float2> spvPerVertex24_1 [[user(locn5)]];
    vertex_value<float2> spvPerVertex24_2 [[user(locn6)]];
    vertex_value<float2> spvPerVertex24_3 [[user(locn7)]];
    vertex_value<float> spvPerVertex24_4 [[user(locn8)]];
    vertex_value<float> spvPerVertex24_5 [[user(locn9)]];
    vertex_value<int2> spvPerVertex106_11 [[user(locn10)]];
    vertex_value<int2> spvPerVertex111_12 [[user(locn10_2)]];
    vertex_value<uint> spvPerVertex127_13 [[user(locn11)]];
    vertex_value<float> spvPerVertex64_6 [[user(locn12)]];
    vertex_value<float2> spvPerVertex147_16 [[user(locn13)]];
    vertex_value<float2> spvPerVertex147_17 [[user(locn14)]];
    vertex_value<float2> spvPerVertex147_18 [[user(locn15)]];
    vertex_value<float2> spvPerVertex147_19 [[user(locn16)]];
    vertex_value<float2> spvPerVertex156_20 [[user(locn17)]];
    vertex_value<float2> spvPerVertex156_21 [[user(locn18)]];
    vertex_value<float2> spvPerVertex156_22 [[user(locn19)]];
    vertex_value<float2> spvPerVertex156_23 [[user(locn20)]];
    vertex_value<float2> spvPerVertex135_14 [[user(locn21)]];
    vertex_value<float> spvPerVertex135_15 [[user(locn24)]];
};

fragment main0_out main0(main0_in in [[stage_in]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    spvUnsafeArray<Inputs, 3> inputs = {};
    spvUnsafeArray<float, 2> pair = {};
    spvUnsafeArray<spvUnsafeArray<float2, 2>, 3> texcoord = {};
    spvUnsafeArray<float2x2, 3> transforms = {};
    spvUnsafeArray<int2, 3> low = {};
    spvUnsafeArray<int2, 3> high = {};
    spvUnsafeArray<uint, 1> first = {};
    spvUnsafeArray<Located, 3> located = {};
    spvUnsafeArray<spvUnsafeArray<float2x2, 2>, 3> matrixArray = {};
    spvUnsafeArray<spvUnsafeArray<spvUnsafeArray<float2, 2>, 2>, 3> nestedArray = {};
    inputs[0].color = in.spvPerVertex24_0.get(vertex_index::first);
    inputs[1].color = in.spvPerVertex24_0.get(vertex_index::second);
    inputs[2].color = in.spvPerVertex24_0.get(vertex_index::third);
    inputs[0].basis[0] = in.spvPerVertex24_1.get(vertex_index::first);
    inputs[1].basis[0] = in.spvPerVertex24_1.get(vertex_index::second);
    inputs[2].basis[0] = in.spvPerVertex24_1.get(vertex_index::third);
    inputs[0].basis[1] = in.spvPerVertex24_2.get(vertex_index::first);
    inputs[1].basis[1] = in.spvPerVertex24_2.get(vertex_index::second);
    inputs[2].basis[1] = in.spvPerVertex24_2.get(vertex_index::third);
    inputs[0].nested.value = in.spvPerVertex24_3.get(vertex_index::first);
    inputs[1].nested.value = in.spvPerVertex24_3.get(vertex_index::second);
    inputs[2].nested.value = in.spvPerVertex24_3.get(vertex_index::third);
    inputs[0].nested.weights[0] = in.spvPerVertex24_4.get(vertex_index::first);
    inputs[1].nested.weights[0] = in.spvPerVertex24_4.get(vertex_index::second);
    inputs[2].nested.weights[0] = in.spvPerVertex24_4.get(vertex_index::third);
    inputs[0].nested.weights[1] = in.spvPerVertex24_5.get(vertex_index::first);
    inputs[1].nested.weights[1] = in.spvPerVertex24_5.get(vertex_index::second);
    inputs[2].nested.weights[1] = in.spvPerVertex24_5.get(vertex_index::third);
    pair[0] = in.spvPerVertex64_6.get(vertex_index::first);
    pair[1] = in.spvPerVertex64_6.get(vertex_index::second);
    texcoord[0][0] = in.spvPerVertex76_7.get(vertex_index::first);
    texcoord[1][0] = in.spvPerVertex76_7.get(vertex_index::second);
    texcoord[2][0] = in.spvPerVertex76_7.get(vertex_index::third);
    texcoord[0][1] = in.spvPerVertex76_8.get(vertex_index::first);
    texcoord[1][1] = in.spvPerVertex76_8.get(vertex_index::second);
    texcoord[2][1] = in.spvPerVertex76_8.get(vertex_index::third);
    transforms[0][0] = in.spvPerVertex84_9.get(vertex_index::first);
    transforms[1][0] = in.spvPerVertex84_9.get(vertex_index::second);
    transforms[2][0] = in.spvPerVertex84_9.get(vertex_index::third);
    transforms[0][1] = in.spvPerVertex84_10.get(vertex_index::first);
    transforms[1][1] = in.spvPerVertex84_10.get(vertex_index::second);
    transforms[2][1] = in.spvPerVertex84_10.get(vertex_index::third);
    low[0] = in.spvPerVertex106_11.get(vertex_index::first);
    low[1] = in.spvPerVertex106_11.get(vertex_index::second);
    low[2] = in.spvPerVertex106_11.get(vertex_index::third);
    high[0] = in.spvPerVertex111_12.get(vertex_index::first);
    high[1] = in.spvPerVertex111_12.get(vertex_index::second);
    high[2] = in.spvPerVertex111_12.get(vertex_index::third);
    first[0] = in.spvPerVertex127_13.get(vertex_index::first);
    located[0].a = in.spvPerVertex135_14.get(vertex_index::first);
    located[1].a = in.spvPerVertex135_14.get(vertex_index::second);
    located[2].a = in.spvPerVertex135_14.get(vertex_index::third);
    located[0].b = in.spvPerVertex135_15.get(vertex_index::first);
    located[1].b = in.spvPerVertex135_15.get(vertex_index::second);
    located[2].b = in.spvPerVertex135_15.get(vertex_index::third);
    matrixArray[0][0][0] = in.spvPerVertex147_16.get(vertex_index::first);
    matrixArray[1][0][0] = in.spvPerVertex147_16.get(vertex_index::second);
    matrixArray[2][0][0] = in.spvPerVertex147_16.get(vertex_index::third);
    matrixArray[0][0][1] = in.spvPerVertex147_17.get(vertex_index::first);
    matrixArray[1][0][1] = in.spvPerVertex147_17.get(vertex_index::second);
    matrixArray[2][0][1] = in.spvPerVertex147_17.get(vertex_index::third);
    matrixArray[0][1][0] = in.spvPerVertex147_18.get(vertex_index::first);
    matrixArray[1][1][0] = in.spvPerVertex147_18.get(vertex_index::second);
    matrixArray[2][1][0] = in.spvPerVertex147_18.get(vertex_index::third);
    matrixArray[0][1][1] = in.spvPerVertex147_19.get(vertex_index::first);
    matrixArray[1][1][1] = in.spvPerVertex147_19.get(vertex_index::second);
    matrixArray[2][1][1] = in.spvPerVertex147_19.get(vertex_index::third);
    nestedArray[0][0][0] = in.spvPerVertex156_20.get(vertex_index::first);
    nestedArray[1][0][0] = in.spvPerVertex156_20.get(vertex_index::second);
    nestedArray[2][0][0] = in.spvPerVertex156_20.get(vertex_index::third);
    nestedArray[0][0][1] = in.spvPerVertex156_21.get(vertex_index::first);
    nestedArray[1][0][1] = in.spvPerVertex156_21.get(vertex_index::second);
    nestedArray[2][0][1] = in.spvPerVertex156_21.get(vertex_index::third);
    nestedArray[0][1][0] = in.spvPerVertex156_22.get(vertex_index::first);
    nestedArray[1][1][0] = in.spvPerVertex156_22.get(vertex_index::second);
    nestedArray[2][1][0] = in.spvPerVertex156_22.get(vertex_index::third);
    nestedArray[0][1][1] = in.spvPerVertex156_23.get(vertex_index::first);
    nestedArray[1][1][1] = in.spvPerVertex156_23.get(vertex_index::second);
    nestedArray[2][1][1] = in.spvPerVertex156_23.get(vertex_index::third);
    int _57 = spvSMod(int(gl_FragCoord.x), 3);
    int _185 = spvSMod(_57, 2);
    out.outputColor = float4(inputs[_57].color + float3(inputs[_57].basis[1], inputs[_57].nested.weights[_185]), pair[_185]);
    float4 _91 = out.outputColor;
    float2 _93 = _91.xy + (texcoord[_57][_185] + transforms[_57][_185]);
    out.outputColor.x = _93.x;
    out.outputColor.y = _93.y;
    float4 _118 = out.outputColor;
    float2 _120 = _118.xy + (inputs[_57].nested.value + float2(low[_57] + high[_57]));
    out.outputColor.x = _120.x;
    out.outputColor.y = _120.y;
    out.outputColor.w += (float(first[0]) + located[_57].b);
    float4 _167 = out.outputColor;
    float2 _169 = _167.xy + ((matrixArray[_57][_185][0] + nestedArray[_57][1][_185]) + located[_57].a);
    out.outputColor.x = _169.x;
    out.outputColor.y = _169.y;
    return out;
}

