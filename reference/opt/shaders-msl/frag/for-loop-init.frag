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
    out.FragColor = 16;
    for (int _140 = 0; _140 < 25; )
    {
        out.FragColor += 10;
        _140++;
        continue;
    }
    for (int _141 = 1; _141 < 30; )
    {
        out.FragColor += 11;
        _141++;
        continue;
    }
    int _142;
    _142 = 0;
    for (; _142 < 20; )
    {
        out.FragColor += 12;
        _142++;
        continue;
    }
    int _62 = _142 + 3;
    out.FragColor += _62;
    if (_62 == 40)
    {
        for (int _143 = 0; _143 < 40; )
        {
            out.FragColor += 13;
            _143++;
            continue;
        }
        return out;
    }
    out.FragColor += _62;
    int2 _144;
    _144 = int2(0);
    for (; _144.x < 10; )
    {
        out.FragColor += _144.y;
        int2 _139 = _144;
        _139.x = _144.x + 4;
        _144 = _139;
        continue;
    }
    for (int _145 = _62; _145 < 40; )
    {
        out.FragColor += _145;
        _145++;
        continue;
    }
    out.FragColor += _62;
    return out;
}

