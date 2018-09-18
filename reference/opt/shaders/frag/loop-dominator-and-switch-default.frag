#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 fragColor;

vec4 _83;

void main()
{
    mediump int _18 = int(_83.x);
    highp vec4 _85;
    _85 = _83;
    highp vec4 _92;
    for (int _84 = 0; _84 < _18; _85 = _92, _84++)
    {
        highp vec4 _86;
        switch (_18)
        {
            case 0:
            {
                highp vec4 _76 = _85;
                _76.y = 0.0;
                _86 = _76;
                break;
            }
            case 1:
            {
                highp vec4 _78 = _85;
                _78.y = 1.0;
                _86 = _78;
                break;
            }
            default:
            {
                int _87;
                highp vec4 _91;
                _91 = _85;
                _87 = 0;
                mediump int _50;
                for (;;)
                {
                    _50 = _87 + 1;
                    if (_87 < _18)
                    {
                        highp vec4 _74 = _91;
                        _74.y = _91.y + 0.5;
                        _91 = _74;
                        _87 = _50;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                _92 = _91;
                continue;
            }
        }
        highp vec4 _82 = _86;
        _82.y = _86.y + 0.5;
        _92 = _82;
        continue;
    }
    fragColor = _85;
}

