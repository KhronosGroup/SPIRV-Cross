#version 450

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
    float gl_CullDistance[1];
};

struct _21
{
    float _m0;
    vec4 _m1;
};

layout(location = 0) in vec4 _17;
layout(location = 0) out _21 _25[3];

void main()
{
    gl_Position = _17;
    _25[2]._m1 = vec4(-4.0, -9.0, 3.0, 7.0);
}
