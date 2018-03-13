#version 450

layout(binding = 0) uniform sampler2D uTextures[2 * 3 * 1];

layout(location = 1) in vec2 vUV;
layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in int vIndex;

void main()
{
    int _92;
    _92 = 0;
    vec4 values3[2 * 3 * 1];
    for (; _92 < 2; _92++)
    {
        int _96;
        _96 = 0;
        for (; _96 < 3; _96++)
        {
            for (int _98 = 0; _98 < 1; )
            {
                values3[_92 * 3 * 1 + _96 * 1 + _98] = texture(uTextures[_92 * 3 * 1 + _96 * 1 + _98], vUV);
                _98++;
                continue;
            }
        }
    }
    FragColor = ((values3[1 * 3 * 1 + 2 * 1 + 0]) + (values3[0 * 3 * 1 + 2 * 1 + 0])) + (values3[(vIndex + 1) * 3 * 1 + 2 * 1 + vIndex]);
}

