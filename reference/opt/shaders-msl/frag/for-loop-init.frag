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
    int _26;
    int _43;
    int _12;
    int _59;
    int _83;
    int2 _93;
    int2 _139;
    int _62;
    int _128;
    out.FragColor = 16;
    while (true)
    {
        out.FragColor += 10;
    }
    for (int _141 = 1; _141 < 30; _43 = _141 + 1, _141 = _43)
    {
        out.FragColor += 11;
    }
    int _143;
    _143 = 0;
    for (; _143 < 20; _59 = _143 + 1, _143 = _59)
    {
        out.FragColor += 12;
    }
    _62 = _143 + 3;
    out.FragColor += _62;
    if (_62 == 40)
    {
        for (int _144 = 0; _144 < 40; _83 = _144 + 1, _144 = _83)
        {
            out.FragColor += 13;
        }
        return out;
    }
    else
    {
        out.FragColor += _62;
    }
    int2 _145;
    _145 = int2(0);
    for (; _145.x < 10; _139 = _145, _139.x = _145.x + 4, _145 = _139)
    {
        out.FragColor += _145.y;
    }
    for (int _146 = _62; _146 < 40; _128 = _146 + 1, _146 = _128)
    {
        out.FragColor += _146;
    }
    out.FragColor += _62;
    return out;
}

