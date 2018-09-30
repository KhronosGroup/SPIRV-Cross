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
    for (;;)
    {
        switch (as_type<int>(_45.cb0[0u].x))
        {
            case -1:
            {
                bool _7_ladder_break = false;
                switch (as_type<int>(_45.cb0[0u].y))
                {
                    case -2:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(-1, -2)), as_type<int>(0.0));
                        _7_ladder_break = true;
                        break;
                    }
                    case -1:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(-1)), as_type<int>(0.0));
                        _7_ladder_break = true;
                        break;
                    }
                    case 0:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(-1, 0)), as_type<int>(0.0));
                        _7_ladder_break = true;
                        break;
                    }
                    case 1:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(-1, 1)), as_type<int>(0.0));
                        _7_ladder_break = true;
                        break;
                    }
                    case 2:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(-1, 2)), as_type<int>(0.0));
                        _7_ladder_break = true;
                        break;
                    }
                }
                if (_7_ladder_break)
                {
                    break;
                }
                break;
            }
            case 0:
            {
                bool _16_ladder_break = false;
                switch (as_type<int>(_45.cb0[0u].y))
                {
                    case -2:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(0, -2)), as_type<int>(0.0));
                        _16_ladder_break = true;
                        break;
                    }
                    case -1:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(0, -1)), as_type<int>(0.0));
                        _16_ladder_break = true;
                        break;
                    }
                    case 0:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)), as_type<int>(0.0));
                        _16_ladder_break = true;
                        break;
                    }
                    case 1:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(0, 1)), as_type<int>(0.0));
                        _16_ladder_break = true;
                        break;
                    }
                    case 2:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(0, 2)), as_type<int>(0.0));
                        _16_ladder_break = true;
                        break;
                    }
                }
                if (_16_ladder_break)
                {
                    break;
                }
                break;
            }
            case 1:
            {
                bool _24_ladder_break = false;
                switch (as_type<int>(_45.cb0[0u].y))
                {
                    case -2:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(1, -2)), as_type<int>(0.0));
                        _24_ladder_break = true;
                        break;
                    }
                    case -1:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(1, -1)), as_type<int>(0.0));
                        _24_ladder_break = true;
                        break;
                    }
                    case 0:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(1, 0)), as_type<int>(0.0));
                        _24_ladder_break = true;
                        break;
                    }
                    case 1:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(1)), as_type<int>(0.0));
                        _24_ladder_break = true;
                        break;
                    }
                    case 2:
                    {
                        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)) + uint2(int2(1, 2)), as_type<int>(0.0));
                        _24_ladder_break = true;
                        break;
                    }
                }
                if (_24_ladder_break)
                {
                    break;
                }
                break;
            }
            default:
            {
                break;
            }
        }
        out.o0 = t0.read(uint2(as_type<int2>(_45.cb0[0u].zw)), as_type<int>(0.0));
        break;
    }
    return out;
}

