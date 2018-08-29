#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    uint gl_BaseVertex [[base_vertex]];
    uint gl_BaseInstance [[base_instance]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = float4(float(in.gl_BaseVertex), float(in.gl_BaseInstance), 0.0f, 1.0f);
    return out;
}

