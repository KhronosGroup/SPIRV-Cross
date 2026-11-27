static float4 gl_Position;
static float3 gl_TessCoord;
static float4 vPos[4];

struct SPIRV_Cross_Input
{
    float4 vPos : TEXCOORD0;
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[4] : SV_TessFactor;
    float gl_TessLevelInner[2] : SV_InsideTessFactor;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void tese_main()
{
    float4 _30 = gl_TessCoord.x.xxxx;
    gl_Position = lerp(lerp(vPos[0], vPos[1], _30), lerp(vPos[3], vPos[2], _30), gl_TessCoord.y.xxxx);
}

[domain("quad")]
SPIRV_Cross_Output main(const OutputPatch<SPIRV_Cross_Input, 4> stage_input, const SPIRV_Cross_PatchConstant patch_input, float2 gl_TessCoordIn : SV_DomainLocation)
{
    gl_TessCoord = float3(gl_TessCoordIn, 0.0f);
    for (int i = 0; i < 4; i++)
    {
        vPos[i] = stage_input[i].vPos;
    }
    tese_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
