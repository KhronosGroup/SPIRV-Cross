struct B
{
    float a;
    float b;
};

cbuffer _42 : register(b0)
{
    int UBO_some_value : packoffset(c0);
};

void partial_inout(inout float4 x)
{
    x.x = 10.0f;
}

void complete_inout(out float4 x)
{
    x = float4(50.0f, 50.0f, 50.0f, 50.0f);
}

void branchy_inout(inout float4 v)
{
    v.y = 20.0f;
    if (UBO_some_value == 20)
    {
        v = float4(50.0f, 50.0f, 50.0f, 50.0f);
    }
}

void branchy_inout_2(out float4 v)
{
    if (UBO_some_value == 20)
    {
        v = float4(50.0f, 50.0f, 50.0f, 50.0f);
    }
    else
    {
        v = float4(70.0f, 70.0f, 70.0f, 70.0f);
    }
    v.y = 20.0f;
}

void partial_inout(inout B b)
{
    b.b = 40.0f;
}

void frag_main()
{
    float4 a = float4(10.0f, 10.0f, 10.0f, 10.0f);
    float4 param = a;
    partial_inout(param);
    a = param;
    float4 param_1;
    complete_inout(param_1);
    a = param_1;
    float4 param_2 = a;
    branchy_inout(param_2);
    a = param_2;
    float4 param_3;
    branchy_inout_2(param_3);
    a = param_3;
    B b = { 10.0f, 20.0f };
    B param_4 = b;
    partial_inout(param_4);
    b = param_4;
}

void main()
{
    frag_main();
}
