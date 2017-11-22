#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    int FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    int _12;
    int _27;
    int _26;
    int _43;
    int _59;
    int _83;
    int2 _93;
    int2 _139;
    int _62;
    int _128;
    out.FragColor = 16;
    for (int _140 = 0; _140 < 25; _27 = _140 + 1, _140 = _27)
    {
        out.FragColor += 10;
    }
    for (int _141 = 1; _141 < 30; _43 = _141 + 1, _141 = _43)
    {
        out.FragColor += 11;
    }
    int _142;
    _142 = 0;
    for (; _142 < 20; _59 = _142 + 1, _142 = _59)
    {
        out.FragColor += 12;
    }
    _62 = _142 + 3;
    out.FragColor += _62;
    if (_62 == 40)
    {
        for (int _143 = 0; _143 < 40; _83 = _143 + 1, _143 = _83)
        {
            out.FragColor += 13;
        }
        return out;
    }
    else
    {
        out.FragColor += _62;
    }
    int2 _144;
    _144 = int2(0);
    for (; _144.x < 10; _139 = _144, _139.x = _144.x + 4, _144 = _139)
    {
        out.FragColor += _144.y;
    }
    for (int _145 = _62; _145 < 40; _128 = _145 + 1, _145 = _128)
    {
        out.FragColor += _145;
    }
    out.FragColor += _62;
    return out;
}

