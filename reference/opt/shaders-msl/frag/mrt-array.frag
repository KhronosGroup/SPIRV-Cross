#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_in
{
    float4 vB [[user(locn1)]];
    float4 vA [[user(locn0)]];
};

struct main0_out
{
    float4 FragColor [[color(0)]][4];
};

// Implementation of the GLSL mod() function, which is slightly different than Metal fmod()
template<typename Tx, typename Ty>
Tx mod(Tx x, Ty y)
{
    return x - y * floor(x / y);
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor[0] = mod(in.vA, in.vB);
    out.FragColor[1] = in.vA + in.vB;
    out.FragColor[2] = in.vA - in.vB;
    out.FragColor[3] = in.vA * in.vB;
    return out;
}

