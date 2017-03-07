struct Foo
{
    float3 a;
    float3 b;
    float3 c;
};

uniform float4 gl_HalfPixel;

static float4 gl_Position;
static float4 Input2;
static float4 Input4;
static float4 Input0;
static float vLocation0;
static float vLocation1;
static float vLocation2[2];
static Foo vLocation4;
static float vLocation7;

struct SPIRV_Cross_Input
{
    float4 Input2 : TEXCOORD2;
    float4 Input4 : TEXCOORD4;
    float4 Input0 : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : POSITION;
    float vLocation0 : TEXCOORD0;
    float vLocation1 : TEXCOORD1;
    float vLocation2[2] : TEXCOORD2;
    Foo vLocation4 : TEXCOORD4;
    float vLocation7 : TEXCOORD7;
};

void vert_main()
{
    gl_Position = ((float4(1.0f, 1.0f, 1.0f, 1.0f) + Input2) + Input4) + Input0;
    vLocation0 = 0.0f;
    vLocation1 = 1.0f;
    vLocation2[0] = 2.0f;
    vLocation2[1] = 2.0f;
    Foo foo;
    foo.a = float3(1.0f, 1.0f, 1.0f);
    foo.b = float3(1.0f, 1.0f, 1.0f);
    foo.c = float3(1.0f, 1.0f, 1.0f);
    vLocation4 = foo;
    vLocation7 = 7.0f;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    Input2 = stage_input.Input2;
    Input4 = stage_input.Input4;
    Input0 = stage_input.Input0;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.vLocation0 = vLocation0;
    stage_output.vLocation1 = vLocation1;
    stage_output.vLocation2 = vLocation2;
    stage_output.vLocation4 = vLocation4;
    stage_output.vLocation7 = vLocation7;
    stage_output.gl_Position.x = stage_output.gl_Position.x - gl_HalfPixel.x * stage_output.gl_Position.w;
    stage_output.gl_Position.y = stage_output.gl_Position.y + gl_HalfPixel.y * stage_output.gl_Position.w;
    return stage_output;
}
