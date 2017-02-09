#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_in
{
    float vIn3 [[user(locn3)]];
    float vIn2 [[user(locn2)]];
    float4 vIn1 [[user(locn1)]];
    float4 vIn0 [[user(locn0)]];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    bool4 l = bool4(false, true, false, false);
    out.FragColor = float4(l.x ? in.vIn1.x : in.vIn0.x, l.y ? in.vIn1.y : in.vIn0.y, l.z ? in.vIn1.z : in.vIn0.z, l.w ? in.vIn1.w : in.vIn0.w);
    bool f = true;
    out.FragColor = float4(f ? in.vIn3 : in.vIn2);
    out.FragColor = f ? in.vIn0 : in.vIn1;
    out.FragColor = float4(f ? in.vIn2 : in.vIn3);
    return out;
}

