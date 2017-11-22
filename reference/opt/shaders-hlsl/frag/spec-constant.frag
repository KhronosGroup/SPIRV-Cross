static const float a = 1.0f;
static const float b = 2.0f;
static const int c = 3;
static const int d = 4;

struct Foo
{
    float elems[(d + 2)];
};

static float4 FragColor;

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

float _146[(c + 2)];

void frag_main()
{
    float _113 = a + b;
    float vec0[(c + 3)][8];
    Foo foo;
    FragColor = ((float4(_113, _113, _113, _113) + float4(vec0[0][0], vec0[0][0], vec0[0][0], vec0[0][0])) + float4(_146[0], _146[0], _146[0], _146[0])) + float4(foo.elems[c], foo.elems[c], foo.elems[c], foo.elems[c]);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
