#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 v0 [[user(locn0)]];
    float4 v1 [[user(locn1)]];
    float4 gl_Position [[position]];
    float gl_PointSize [[point_size]];
};

vertex main0_out main0()
{
    main0_out out = {};
    float gl_ClipDistance[2] = {};
    out.v0 = float4(1.0);
    out.v1 = float4(2.0);
    out.gl_Position = float4(3.0);
    out.gl_PointSize = 4.0;
    gl_ClipDistance[0] = 1.0;
    gl_ClipDistance[1] = 0.5;
    return out;
}

