#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float a [[user(locn0)]];
    float b [[user(locn1)]];
    float4 va [[user(locn2)]];
    float4 vb [[user(locn3)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = (float4((isordered(in.va, in.vb) && in.va != in.vb)) + float4(in.va != in.vb)) * (float((isordered(in.a, in.b) && in.a != in.b)) + float(in.a != in.b));
    return out;
}

