static float4 gl_PositionIn[3];
static float4 gl_Position;
static float3 gl_TessCoord;
static float2 oUV;
static float2 vUV[3];

struct SPIRV_Cross_Input
{
    float2 vUV : TEXCOORD0;
    float4 gl_PositionIn : SV_Position;
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[3] : SV_TessFactor;
    float gl_TessLevelInner : SV_InsideTessFactor;
};

struct SPIRV_Cross_Output
{
    float2 oUV : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void tese_main()
{
    oUV = ((vUV[0] * gl_TessCoord.x) + (vUV[1] * gl_TessCoord.y)) + (vUV[2] * gl_TessCoord.z);
    gl_Position = ((gl_PositionIn[0] * gl_TessCoord.x) + (gl_PositionIn[1] * gl_TessCoord.y)) + (gl_PositionIn[2] * gl_TessCoord.z);
}

[domain("tri")]
SPIRV_Cross_Output main(const OutputPatch<SPIRV_Cross_Input, 3> stage_input, const SPIRV_Cross_PatchConstant patch_input, float3 gl_TessCoordIn : SV_DomainLocation)
{
    for (int i = 0; i < 3; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
    }
    gl_TessCoord = gl_TessCoordIn;
    for (int i = 0; i < 3; i++)
    {
        vUV[i] = stage_input[i].vUV;
    }
    tese_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.oUV = oUV;
    return stage_output;
}
