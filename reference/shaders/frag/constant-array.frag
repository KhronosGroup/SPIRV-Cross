#version 310 es
precision mediump float;
precision highp int;

struct Foobar
{
    float a;
    float b;
};

layout(location = 0) out vec4 FragColor;
layout(location = 0) in mediump flat int index;

vec4 resolve(Foobar f)
{
    return vec4((f.a + f.b));
}

void main()
{
    highp vec4 indexable[3] = vec4[](vec4(1.000000), vec4(2.000000), vec4(3.000000));
    highp vec4 indexable_1[2][2] = vec4[][](vec4[](vec4(1.000000), vec4(2.000000)), vec4[](vec4(8.000000), vec4(10.000000)));
    Foobar param = Foobar(10.000000, 20.000000);
    Foobar indexable_2[2] = Foobar[](Foobar(10.000000, 40.000000), Foobar(90.000000, 70.000000));
    Foobar param_1 = indexable_2[index];
    FragColor = (((indexable[index] + indexable_1[index][(index + 1)]) + resolve(param)) + resolve(param_1));
}

