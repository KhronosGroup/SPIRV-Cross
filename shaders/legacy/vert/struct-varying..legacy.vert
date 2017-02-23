#version 310 es

struct Output
{
   vec4 a;
   vec2 b;
};

layout(location = 0) out Output vout;

void main()
{
   Output s = Output(vec4(0.5), vec2(0.25));

   // Write whole struct.
   vout = s;
   // Write whole struct again, checks for scoping.
   vout = s;

   // Write elements individually.
   vout.a = s.a;
   vout.b = s.b;
}
