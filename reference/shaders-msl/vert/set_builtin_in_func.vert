#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

void write_position(thread float4& gl_Position)
{
    gl_Position = float4(1.0);
}

vertex main0_out main0()
{
    main0_out out = {};
    write_position(out.gl_Position);
    return out;
}

