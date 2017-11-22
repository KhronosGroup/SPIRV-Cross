#version 100
precision mediump float;
precision highp int;

struct Inputs
{
    highp vec4 a;
    highp vec2 b;
};

varying highp vec4 Inputs_a;
varying highp vec2 Inputs_b;

void main()
{
    gl_FragData[0] = ((((Inputs(Inputs_a, Inputs_b).a + Inputs(Inputs_a, Inputs_b).b.xxyy) + Inputs(Inputs_a, Inputs_b).a) + Inputs(Inputs_a, Inputs_b).b.yyxx) + Inputs_a) + Inputs_b.xxyy;
}

