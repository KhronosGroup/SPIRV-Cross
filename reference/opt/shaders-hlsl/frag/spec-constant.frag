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

void frag_main()
{
    Foo foo;
    foo.elems[c] = 10.0f;
    FragColor = ((a + b).xxxx + 30.0f.xxxx) + foo.elems[c].xxxx;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
