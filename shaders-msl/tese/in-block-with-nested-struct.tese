#version 450
layout(triangles, ccw, equal_spacing) in;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[1];
    float gl_CullDistance[1];
};

struct _35
{
    vec2 _m0;
    vec4 _m1;
};

layout(location = 0) in _36
{
    vec2 _m0;
    _35 _m1;
} _40[32];

layout(location = 0) out float _80;

void main()
{
    gl_Position = vec4((gl_TessCoord.xy * 2.0) - vec2(1.0), 0.0, 1.0);
    float _34 = ((float(abs(_40[0]._m1._m1.x - (-4.0)) < 0.001000000047497451305389404296875) * float(abs(_40[0]._m1._m1.y - (-9.0)) < 0.001000000047497451305389404296875)) * float(abs(_40[0]._m1._m1.z - 3.0) < 0.001000000047497451305389404296875)) * float(abs(_40[0]._m1._m1.w - 7.0) < 0.001000000047497451305389404296875);
    _80 = _34;
}
