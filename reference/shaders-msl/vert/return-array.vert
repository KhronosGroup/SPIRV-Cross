#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

float4[2] test()
{
    return {float4(10.0), float4(20.0)};
}

vertex main0_out main0()
{
    main0_out out = {};
    out.gl_Position = test()[0];
    return out;
}

