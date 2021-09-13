struct Foo
{
    float4 a;
    float4 b;
};
static float4 gl_Position;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
static float3 gl_TessCoord;
static float4 vColors;
static Foo vFoo;
static Foo vFooArray[2];
static float4 vColorsArray[2];
struct SPIRV_Cross_DS_Point_Input
{
    float4 vColor : TEXCOORD0;
    Foo vFoos : TEXCOORD14;
};
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};
struct SPIRV_Cross_DS_Constant_Input
{
    float4 vColors : TEXCOORD1;
    float4 vColorsArray[2] : TEXCOORD2;
    Foo vFoo : TEXCOORD8;
    Foo vFooArray[2] : TEXCOORD10;
    float gl_TessLevelOuter[4] : SV_TessFactor;
    float gl_TessLevelInner[2] : SV_InsideTessFactor;
    float2 gl_TessCoord : SV_DomainLocation;
};
void set_from_function(OutputPatch<SPIRV_Cross_DS_Point_Input,32> gl_in)
{
    gl_Position += gl_in[0].vColor;
    gl_Position += gl_in[1].vColor;
    gl_Position += vColors;
    Foo foo = vFoo;
    gl_Position += foo.a;
    gl_Position += foo.b;
    foo = vFooArray[0];
    gl_Position += foo.a;
    gl_Position += foo.b;
    foo = vFooArray[1];
    gl_Position += foo.a;
    gl_Position += foo.b;
    foo = gl_in[0].vFoos;
    gl_Position += foo.a;
    gl_Position += foo.b;
    foo = gl_in[1].vFoos;
    gl_Position += foo.a;
    gl_Position += foo.b;
}
void tese_main(OutputPatch<SPIRV_Cross_DS_Point_Input,32> gl_in)
{
    set_from_function(gl_in);
}
[domain("quad")]
SPIRV_Cross_Output main(SPIRV_Cross_DS_Constant_Input stage_input, OutputPatch<SPIRV_Cross_DS_Point_Input, 32> gl_in)
{
    gl_TessLevelOuter = stage_input.gl_TessLevelOuter;
    gl_TessLevelInner = stage_input.gl_TessLevelInner;
    gl_TessCoord = float3(stage_input.gl_TessCoord, 0.0);
    vColors = stage_input.vColors;
    vFoo = stage_input.vFoo;
    vFooArray = stage_input.vFooArray;
    vColorsArray = stage_input.vColorsArray;
    tese_main(gl_in);
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}