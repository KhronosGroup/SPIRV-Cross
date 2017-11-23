#version 450

layout(binding = 0) uniform sampler2D uTextures[2 * 3 * 1];

layout(location = 1) in vec2 vUV;
layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in int vIndex;

int _93;

void main()
{
    int _9;
    int _76;
    int _74;
    int _72;
    vec4 values3[2 * 3 * 1];
    int _96;
    int _97;
    for (int _92 = 0, _94 = _93, _95 = _93; _92 < 2; _76 = _92 + 1, _92 = _76, _94 = _96, _95 = _97)
    {
        _96 = 0;
        _97 = _95;
        int _98;
        for (; _96 < 3; _74 = _96 + 1, _96 = _74, _97 = _98)
        {
            _98 = 0;
            for (; _98 < 1; _72 = _98 + 1, _98 = _72)
            {
                values3[_92 * 3 * 1 + _96 * 1 + _98] = texture(uTextures[_92 * 3 * 1 + _96 * 1 + _98], vUV);
            }
        }
    }
    FragColor = ((values3[1 * 3 * 1 + 2 * 1 + 0]) + (values3[0 * 3 * 1 + 2 * 1 + 0])) + (values3[(vIndex + 1) * 3 * 1 + 2 * 1 + vIndex]);
}

