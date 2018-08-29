#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    uint gl_BaseVertex;
    uint gl_BaseInstance;
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = float4(in.gl_BaseVertex, in.gl_BaseInstance, 0, 1);
    return out;
}

