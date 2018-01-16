#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int counter;

void main()
{
    int _37;
    mediump int _22;
    uint _38;
    mediump uint _25;
    FragColor = vec4(0.0);
    int _53 = 0;
    uint _54 = 1u;
    for (; (_53 < 10) && (int(_54) < int(20u)); _22 = _53 + counter, _25 = _54 + uint(counter), _53 = _22, _54 = _25)
    {
        FragColor += vec4(float(_53));
        FragColor += vec4(float(_54));
    }
}

