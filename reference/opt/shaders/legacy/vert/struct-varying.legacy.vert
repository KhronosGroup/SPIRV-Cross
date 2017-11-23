#version 100

struct Output
{
    vec4 a;
    vec2 b;
};

varying vec4 Output_a;
varying vec2 Output_b;

void main()
{
    {
        Output vout = Output(vec4(0.5), vec2(0.25));
        Output_a = vout.a;
        Output_b = vout.b;
    }
    {
        Output vout = Output(vec4(0.5), vec2(0.25));
        Output_a = vout.a;
        Output_b = vout.b;
    }
    Output _22 = Output(Output_a, Output_b);
    Output_a = _22.a;
    Output_b = _22.b;
    Output_a.x = 1.0;
    Output_b.y = 1.0;
}

