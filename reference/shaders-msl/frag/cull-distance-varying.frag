#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float gl_CullDistance_0 [[user(cull0)]];
    float gl_CullDistance_1 [[user(cull1)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float gl_CullDistance[2] = {};
    gl_CullDistance[0] = in.gl_CullDistance_0;
    gl_CullDistance[1] = in.gl_CullDistance_1;
    out.FragColor = float4((1.0 - gl_CullDistance[0]) - gl_CullDistance[1]);
    return out;
}

