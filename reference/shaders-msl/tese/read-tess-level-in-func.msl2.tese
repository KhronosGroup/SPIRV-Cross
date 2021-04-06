#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_patchIn
{
    float4 gl_TessLevel [[attribute(0)]];
};

static inline __attribute__((always_inline))
float4 read_tess_levels(thread float (&gl_TessLevelOuter)[4])
{
    return float4(gl_TessLevelOuter[0], gl_TessLevelOuter[1], gl_TessLevelOuter[2], gl_TessLevelOuter[3]);
}

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    float gl_TessLevelOuter[4] = {};
    gl_TessLevelOuter[0] = patchIn.gl_TessLevel.x;
    gl_TessLevelOuter[1] = patchIn.gl_TessLevel.y;
    gl_TessLevelOuter[2] = patchIn.gl_TessLevel.z;
    out.gl_Position = read_tess_levels(gl_TessLevelOuter);
    return out;
}

