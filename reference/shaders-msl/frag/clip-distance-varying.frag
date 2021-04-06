#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float gl_ClipDistance_0 [[user(clip0)]];
    float gl_ClipDistance_1 [[user(clip1)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float gl_ClipDistance[2] = {};
    gl_ClipDistance[0] = in.gl_ClipDistance_0;
    gl_ClipDistance[1] = in.gl_ClipDistance_1;
    out.FragColor = float4((1.0 - gl_ClipDistance[0]) - gl_ClipDistance[1]);
    return out;
}

