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
    float4 _35;
    if (f)
    {
        _35 = in.vIn0;
    }
    else
    {
        _35 = in.vIn1;
    }
    out.FragColor = _35;
    float _44;
    if (f)
    {
        _44 = in.vIn2;
    }
    else
    {
        _44 = in.vIn3;
    }
    out.FragColor = float4(_44);
    return out;
}

