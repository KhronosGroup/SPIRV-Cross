#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct ResType
{
    half4 _m0;
    int4 _m1;
};

struct main0_in
{
    half4 v4 [[user(locn3)]];
};

fragment void main0(main0_in in [[stage_in]])
{
    half4 _491;
    half4 _563 = modf(in.v4, _491);
}

