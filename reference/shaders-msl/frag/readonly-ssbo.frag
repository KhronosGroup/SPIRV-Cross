#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SSBO
{
    float4 v;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
float4 read_from_function(const device SSBO& _13)
{
    return _13.v;
}

fragment main0_out main0(const device SSBO& _13 [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = _13.v + read_from_function(_13);
    return out;
}

