static float4 gl_Position;
static uint gl_PrimitiveID;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
static float3 gl_TessCoord;
static int gl_PatchVerticesIn;
static float4 vPos[32];
static float4 oColor;
static float4 vColor;

struct SPIRV_Cross_Input
{
    float4 vPos : TEXCOORD0;
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[4] : SV_TessFactor;
    float gl_TessLevelInner[2] : SV_InsideTessFactor;
    float4 vColor : PATCH1;
};

struct SPIRV_Cross_Output
{
    float4 oColor : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void tese_main()
{
    float4 a = lerp(vPos[0], vPos[1], gl_TessCoord.x.xxxx);
    float4 b = lerp(vPos[3], vPos[2], gl_TessCoord.x.xxxx);
    gl_Position = lerp(a, b, gl_TessCoord.y.xxxx);
    oColor = (vColor * (gl_TessLevelOuter[0] + gl_TessLevelInner[1])) + float4(float(gl_PrimitiveID), float(gl_PatchVerticesIn), 0.0f, 0.0f);
}

[domain("quad")]
SPIRV_Cross_Output main(const OutputPatch<SPIRV_Cross_Input, 4> stage_input, const SPIRV_Cross_PatchConstant patch_input, float2 gl_TessCoordIn : SV_DomainLocation, uint gl_PrimitiveIDIn : SV_PrimitiveID)
{
    gl_PrimitiveID = gl_PrimitiveIDIn;
    gl_TessLevelOuter[0] = patch_input.gl_TessLevelOuter[0];
    gl_TessLevelOuter[1] = patch_input.gl_TessLevelOuter[1];
    gl_TessLevelOuter[2] = patch_input.gl_TessLevelOuter[2];
    gl_TessLevelOuter[3] = patch_input.gl_TessLevelOuter[3];
    gl_TessLevelInner[0] = patch_input.gl_TessLevelInner[0];
    gl_TessLevelInner[1] = patch_input.gl_TessLevelInner[1];
    gl_TessCoord = float3(gl_TessCoordIn, 0.0f);
    gl_PatchVerticesIn = 4;
    for (int i = 0; i < 4; i++)
    {
        vPos[i] = stage_input[i].vPos;
    }
    vColor = patch_input.vColor;
    tese_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.oColor = oColor;
    return stage_output;
}
