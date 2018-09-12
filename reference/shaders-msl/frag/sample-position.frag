#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(uint gl_SampleID [[sample_id]])
{
    main0_out out = {};
    out.FragColor = float4(get_sample_position(gl_SampleID), float(gl_SampleID), 1.0);
    return out;
}

