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
    vec4 _m0;
    vec4 _m1;
};

layout(location = 0) in vec4 _17;
layout(location = 0) out _24
{
    _21 _m0[3];
} _26;


void main()
{
    gl_Position = _17;
    _26._m0[1]._m1 = vec4(-4.0, -9.0, 3.0, 7.0);
}
