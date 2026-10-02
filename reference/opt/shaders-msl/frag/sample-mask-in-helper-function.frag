#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 o [[color(0)]];
};

fragment main0_out main0(uint gl_SampleMaskIn [[sample_mask]])
{
    main0_out out = {};
    out.o = float4(float(int(gl_SampleMaskIn) & 1));
    return out;
}

