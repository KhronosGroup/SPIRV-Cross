#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 v0;

int _51;

void main()
{
    FragColor = vec4(1.0);
    int _53;
    int _52;
    for (int _50 = 0; _50 < 4; _50++, _52 = _53)
    {
        _53 = 0;
        for (; _53 < 4; _53++)
        {
            FragColor += vec4(v0[(_50 + _53) & 3]);
        }
    }
}

