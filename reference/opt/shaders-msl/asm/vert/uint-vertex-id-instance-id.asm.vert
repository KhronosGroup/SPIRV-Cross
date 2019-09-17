#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0(uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]], uint gl_BaseVertex [[base_vertex]], uint gl_BaseInstance [[base_instance]])
{
    main0_out out = {};
    out.gl_Position = float4(float((gl_VertexIndex - gl_BaseVertex) + (gl_InstanceIndex - gl_BaseInstance)));
    return out;
}

