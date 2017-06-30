#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float a_tmp [[function_constant(1)]];
constant float a = is_function_constant_defined(a_tmp) ? a_tmp : 1.0;
constant float b_tmp [[function_constant(2)]];
constant float b = is_function_constant_defined(b_tmp) ? b_tmp : 2.0;
constant int c_tmp [[function_constant(3)]];
constant int c = is_function_constant_defined(c_tmp) ? c_tmp : 3;
constant int d_tmp [[function_constant(4)]];
constant int d = is_function_constant_defined(d_tmp) ? d_tmp : 4;
constant uint e_tmp [[function_constant(5)]];
constant uint e = is_function_constant_defined(e_tmp) ? e_tmp : 5u;
constant uint f_tmp [[function_constant(6)]];
constant uint f = is_function_constant_defined(f_tmp) ? f_tmp : 6u;
constant bool g_tmp [[function_constant(7)]];
constant bool g = is_function_constant_defined(g_tmp) ? g_tmp : false;
constant bool h_tmp [[function_constant(8)]];
constant bool h = is_function_constant_defined(h_tmp) ? h_tmp : true;

struct main0_out
{
    half4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    half t0 = a;
    half t1 = b;
    ushort c0 = (ushort(c) + 0u);
    short c1 = (-c);
    short c2 = (~c);
    short c3 = (c + d);
    short c4 = (c - d);
    short c5 = (c * d);
    short c6 = (c / d);
    ushort c7 = (e / f);
    short c8 = (c % d);
    ushort c9 = (e % f);
    short c10 = (c >> d);
    ushort c11 = (e >> f);
    short c12 = (c << d);
    short c13 = (c | d);
    short c14 = (c ^ d);
    short c15 = (c & d);
    bool c16 = (g || h);
    bool c17 = (g && h);
    bool c18 = (!g);
    bool c19 = (g == h);
    bool c20 = (g != h);
    bool c21 = (c == d);
    bool c22 = (c != d);
    bool c23 = (c < d);
    bool c24 = (e < f);
    bool c25 = (c > d);
    bool c26 = (e > f);
    bool c27 = (c <= d);
    bool c28 = (e <= f);
    bool c29 = (c >= d);
    bool c30 = (e >= f);
    short c31 = c8 + c3;
    short c32 = int(e + 0u);
    bool c33 = (c != int(0u));
    bool c34 = (e != 0u);
    short c35 = int(g);
    ushort c36 = uint(g);
    half c37 = float(g);
    out.FragColor = half4(t0 + t1);
    return out;
}

