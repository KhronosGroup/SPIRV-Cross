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
    Output s = Output(vec4(0.5), vec2(0.25));
    {
        Output vout = s;
        Output_a = vout.a;
        Output_b = vout.b;
    }
    {
        Output vout = s;
        Output_a = vout.a;
        Output_b = vout.b;
    }
    Output_a = s.a;
    Output_b = s.b;
}

