#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct push_cb
{
    float4 cb0[1];
};

struct main0_out
{
    float4 o0 [[color(0)]];
};

fragment main0_out main0(constant push_cb& _45 [[buffer(0)]], texture2d<float> t0 [[texture(2)]], sampler dummy_sampler [[sampler(4)]])
{
    main0_out out = {};
    float4 r0;
    switch (as_type<int>(_45.cb0[0u].x))
    {
        case -1:
        {
            switch (as_type<int>(_45.cb0[0u].y))
            {
                case -2:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(-1, -2)), as_type<int>(r0.w));
                    return out;
                }
                case -1:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(-1)), as_type<int>(r0.w));
                    return out;
                }
                case 0:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(-1, 0)), as_type<int>(r0.w));
                    return out;
                }
                case 1:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(-1, 1)), as_type<int>(r0.w));
                    return out;
                }
                case 2:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(-1, 2)), as_type<int>(r0.w));
                    return out;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case 0:
        {
            switch (as_type<int>(_45.cb0[0u].y))
            {
                case -2:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(0, -2)), as_type<int>(r0.w));
                    return out;
                }
                case -1:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(0, -1)), as_type<int>(r0.w));
                    return out;
                }
                case 0:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)), as_type<int>(r0.w));
                    return out;
                }
                case 1:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(0, 1)), as_type<int>(r0.w));
                    return out;
                }
                case 2:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(0, 2)), as_type<int>(r0.w));
                    return out;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case 1:
        {
            switch (as_type<int>(_45.cb0[0u].y))
            {
                case -2:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(1, -2)), as_type<int>(r0.w));
                    return out;
                }
                case -1:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(1, -1)), as_type<int>(r0.w));
                    return out;
                }
                case 0:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(1, 0)), as_type<int>(r0.w));
                    return out;
                }
                case 1:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(1)), as_type<int>(r0.w));
                    return out;
                }
                case 2:
                {
                    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
                    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
                    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)) + uint2(int2(1, 2)), as_type<int>(r0.w));
                    return out;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
    r0 = float4(_45.cb0[0u].z, _45.cb0[0u].w, r0.z, r0.w);
    r0 = float4(r0.x, r0.y, float2(0.0).x, float2(0.0).y);
    out.o0 = t0.read(uint2(as_type<int2>(r0.xy)), as_type<int>(r0.w));
    return out;
}

