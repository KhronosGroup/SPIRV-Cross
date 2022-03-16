#version 320 es
precision mediump float;
precision highp int;

struct foo
{
    float a[3];
};

void main()
{
    foo a = foo(float[](1.0, 2.0, 3.0));
    float b[3] = a.a;
}

