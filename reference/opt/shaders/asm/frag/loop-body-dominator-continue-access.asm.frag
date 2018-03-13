#version 450

layout(binding = 0, std140) uniform Foo
{
    layout(row_major) mat4 lightVP[64];
    uint shadowCascadesNum;
    int test;
} _11;

layout(location = 0) in vec3 fragWorld;
layout(location = 0) out int _entryPointOutput;

int GetCascade(vec3 fragWorldPosition)
{
    for (uint _151 = 0u; _151 < _11.shadowCascadesNum; _151++)
    {
        mat4 _157;
        for (;;)
        {
            if (_11.test == 0)
            {
                _157 = mat4(vec4(0.5, 0.0, 0.0, 0.0), vec4(0.0, 0.5, 0.0, 0.0), vec4(0.0, 0.0, 0.5, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
                break;
            }
            _157 = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
            break;
        }
        vec4 _92 = (_157 * _11.lightVP[_151]) * vec4(fragWorldPosition, 1.0);
        float _140 = _92.z;
        float _144 = _92.x;
        float _146 = _92.y;
        if ((((_140 >= 0.0) && (_140 <= 1.0)) && (max(_144, _146) <= 1.0)) && (min(_144, _146) >= 0.0))
        {
            return int(_151);
        }
        else
        {
            continue;
        }
        continue;
    }
    return -1;
}

void main()
{
    vec3 _123 = fragWorld;
    _entryPointOutput = GetCascade(_123);
}

