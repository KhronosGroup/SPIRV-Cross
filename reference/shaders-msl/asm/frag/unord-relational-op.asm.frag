#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float a_tmp [[function_constant(1)]];
constant float a = is_function_constant_defined(a_tmp) ? a_tmp : 1.0;
constant float b_tmp [[function_constant(2)]];
constant float b = is_function_constant_defined(b_tmp) ? b_tmp : 2.0;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    float t0 = a;
    float t1 = b;
    bool c21 = (isunordered(a, b) || a == b);
    bool c22 = (isunordered(a, b) || a != b);
    bool c23 = (isunordered(a, b) || a < b);
    bool c25 = (isunordered(a, b) || a > b);
    bool c27 = (isunordered(a, b) || a <= b);
    bool c29 = (isunordered(a, b) || a >= b);
    out.FragColor = float4(t0 + t1);
    return out;
}

