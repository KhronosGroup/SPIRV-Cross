#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int vA;
layout(location = 1) flat in mediump int vB;

void main()
{
    FragColor = vec4(0.0);
    int _60;
    for (int _57 = 0, _58 = 0; _58 < vA; FragColor += vec4(1.0), _57 = _60, _58 += (_60 + 10))
    {
        if ((vA + _58) == 20)
        {
            _60 = 50;
            continue;
        }
        else
        {
            _60 = ((vB + _58) == 40) ? 60 : _57;
            continue;
        }
        continue;
    }
}

