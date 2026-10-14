#version 450
layout(location = 0) out vec4 o;
float f() { return float(gl_SampleMaskIn[0] & 1); }
void main() { o = vec4(f()); }
