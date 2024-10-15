#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float3 value [[color(0)]];
};

struct main0_in
{
    float3 gl_BaryCoordNoPerspEXT [[barycentric_coord, sample_no_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.value = in.gl_BaryCoordNoPerspEXT;
    return out;
}

