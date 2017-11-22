#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out mediump int FragColor;

void main()
{
    int _12;
    mediump int _27;
    int _26;
    mediump int _43;
    mediump int _59;
    mediump int _83;
    ivec2 _93;
    ivec2 _139;
    mediump int _62;
    mediump int _128;
    FragColor = 16;
    for (int _140 = 0; _140 < 25; _27 = _140 + 1, _140 = _27)
    {
        FragColor += 10;
    }
    for (int _141 = 1; _141 < 30; _43 = _141 + 1, _141 = _43)
    {
        FragColor += 11;
    }
    int _142;
    _142 = 0;
    for (; _142 < 20; _59 = _142 + 1, _142 = _59)
    {
        FragColor += 12;
    }
    _62 = _142 + 3;
    FragColor += _62;
    if (_62 == 40)
    {
        for (int _143 = 0; _143 < 40; _83 = _143 + 1, _143 = _83)
        {
            FragColor += 13;
        }
        return;
    }
    else
    {
        FragColor += _62;
    }
    ivec2 _144;
    _144 = ivec2(0);
    for (; _144.x < 10; _139 = _144, _139.x = _144.x + 4, _144 = _139)
    {
        FragColor += _144.y;
    }
    for (int _145 = _62; _145 < 40; _128 = _145 + 1, _145 = _128)
    {
        FragColor += _145;
    }
    FragColor += _62;
}

