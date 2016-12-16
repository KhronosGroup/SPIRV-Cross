#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out mediump int FragColor;

void main()
{
    FragColor = 15;
    for (mediump int i = 0; i < 25; i = i + 1)
    {
        FragColor = FragColor + 10;
    }
    for (mediump int j = 4, i_1 = 1; i_1 < 30; i_1 = i_1 + 1, j = j + 4)
    {
        FragColor = FragColor + 11;
    }
    mediump int k = 0;
    for (; k < 20; k = k + 1)
    {
        FragColor = FragColor + 12;
    }
    k = k + 3;
    FragColor = FragColor + k;
    mediump int l;
    if (k == 40)
    {
        l = 0;
        for (; l < 40; l = l + 1)
        {
            FragColor = FragColor + 13;
        }
        return;
    }
    else
    {
        l = k;
        FragColor = FragColor + l;
    }
    mediump ivec2 i_2 = ivec2(0);
    for (; i_2.x < 10; i_2.x = i_2.x + 1)
    {
        FragColor = FragColor + i_2.y;
    }
    mediump int o = k;
    for (mediump int m = k; m < 40; m = m + 1)
    {
        FragColor = FragColor + m;
    }
    FragColor = FragColor + o;
}

