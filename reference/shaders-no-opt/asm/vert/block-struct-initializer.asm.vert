#version 450

struct Foo
{
    float c;
    float d;
};

layout(location = 0) out Vert
{
    float a;
    float b;
} _3;

layout(location = 2) out Foo foo;

void main()
{
    _3.a = 0.0;
    _3.b = 0.0;
    foo = Foo(0.0, 0.0);
}

