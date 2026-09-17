static float4 gl_Position;
static float3 gl_TessCoord;
static float3 outColor;

struct SPIRV_Cross_Input
{
};

struct SPIRV_Cross_Output
{
    float3 outColor : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[3] : SV_TessFactor;
    float gl_TessLevelInner[1] : SV_InsideTessFactor;
};

void tese_main()
{
    gl_Position = float4(gl_TessCoord, 1.0f);
    outColor = gl_TessCoord;
}

[domain("tri")]
SPIRV_Cross_Output main(SPIRV_Cross_PatchConstant pc, const OutputPatch<SPIRV_Cross_Input, 3> patch, float3 domain : SV_DomainLocation)
{
    gl_TessCoord = domain;
    tese_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.outColor = outColor;
    return stage_output;
}
