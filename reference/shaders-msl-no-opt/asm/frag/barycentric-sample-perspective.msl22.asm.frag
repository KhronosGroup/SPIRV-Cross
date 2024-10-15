#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float3 value [[color(0)]];
};

struct main0_in
{
    float3 gl_BaryCoordEXT [[barycentric_coord, sample_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.value = in.gl_BaryCoordEXT;
    return out;
}

