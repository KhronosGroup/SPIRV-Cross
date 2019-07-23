#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out mediump int FragColor;

void main()
{
    mediump int _145;
    for (;;)
    {
        FragColor = 16;
        _145 = 0;
        for (; _145 < 25; )
        {
            FragColor += 10;
            _145++;
            continue;
        }
        for (mediump int _146 = 1; _146 < 30; )
        {
            FragColor += 11;
            _146++;
            continue;
        }
        mediump int _147;
        _147 = 0;
        for (; _147 < 20; )
        {
            FragColor += 12;
            _147++;
            continue;
        }
        mediump int _62 = _147 + 3;
        FragColor += _62;
        if (_62 == 40)
        {
            for (mediump int _151 = 0; _151 < 40; )
            {
                FragColor += 13;
                _151++;
                continue;
            }
            break;
        }
        FragColor += _62;
        mediump ivec2 _148;
        _148 = ivec2(0);
        mediump int _102;
        for (;;)
        {
            _102 = _148.x;
            if (_102 < 10)
            {
                FragColor += _148.y;
                mediump ivec2 _144 = _148;
                _144.x = _102 + 4;
                _148 = _144;
                continue;
            }
            else
            {
                break;
            }
        }
        for (mediump int _150 = _62; _150 < 40; )
        {
            FragColor += _150;
            _150++;
            continue;
        }
        FragColor += _62;
        break;
    }
}

