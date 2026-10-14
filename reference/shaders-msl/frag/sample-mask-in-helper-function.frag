#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 o [[color(0)]];
};

static inline __attribute__((always_inline))
float f(uint gl_SampleMaskIn)
{
    return float(int(gl_SampleMaskIn) & 1);
}

fragment main0_out main0(uint gl_SampleMaskIn [[sample_mask]])
{
    main0_out out = {};
    out.o = float4(f(gl_SampleMaskIn));
    return out;
}

