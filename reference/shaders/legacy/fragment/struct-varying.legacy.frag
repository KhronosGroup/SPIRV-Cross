#version 100
precision mediump float;
precision highp int;

struct Inputs
{
    vec4 a;
    vec2 b;
};

varying vec4 Inputs_a;
varying vec2 Inputs_b;

void main()
{
    Inputs v0 = Inputs(Inputs_a, Inputs_b);
    Inputs v1 = Inputs(Inputs_a, Inputs_b);
    vec4 a = Inputs_a;
    vec4 b = Inputs_b.xxyy;
    gl_FragData[0] = ((((v0.a + v0.b.xxyy) + v1.a) + v1.b.yyxx) + a) + b;
}

