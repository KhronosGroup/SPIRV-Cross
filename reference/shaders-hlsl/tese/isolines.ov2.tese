static float4 gl_PositionIn[2];
static float4 gl_Position;
static float3 gl_TessCoord;
struct SPIRV_Cross_Input
{
    float4 gl_PositionIn : SV_Position;
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[2] : SV_TessFactor;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void tese_main()
{
    gl_Position = lerp(gl_PositionIn[0], gl_PositionIn[1], gl_TessCoord.x.xxxx) + float4(0.0f, gl_TessCoord.y, 0.0f, 0.0f);
}

[domain("isoline")]
SPIRV_Cross_Output main(const OutputPatch<SPIRV_Cross_Input, 2> stage_input, const SPIRV_Cross_PatchConstant patch_input, float2 gl_TessCoordIn : SV_DomainLocation)
{
    for (int i = 0; i < 2; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
    }
    gl_TessCoord = float3(gl_TessCoordIn, 0.0f);
    tese_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}
