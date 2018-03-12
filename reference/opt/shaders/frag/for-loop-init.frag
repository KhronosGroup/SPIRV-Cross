#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out mediump int FragColor;

void main()
{
    FragColor = 16;
    for (int _140 = 0; _140 < 25; )
    {
        FragColor += 10;
        _140++;
        continue;
    }
    for (int _141 = 1; _141 < 30; )
    {
        FragColor += 11;
        _141++;
        continue;
    }
    int _142;
    _142 = 0;
    for (; _142 < 20; )
    {
        FragColor += 12;
        _142++;
        continue;
    }
    mediump int _62 = _142 + 3;
    FragColor += _62;
    if (_62 == 40)
    {
        for (int _143 = 0; _143 < 40; )
        {
            FragColor += 13;
            _143++;
            continue;
        }
        return;
    }
    FragColor += _62;
    ivec2 _144;
    _144 = ivec2(0);
    for (; _144.x < 10; )
    {
        FragColor += _144.y;
        ivec2 _139 = _144;
        _139.x = _144.x + 4;
        _144 = _139;
        continue;
    }
    for (int _145 = _62; _145 < 40; )
    {
        FragColor += _145;
        _145++;
        continue;
    }
    FragColor += _62;
}

