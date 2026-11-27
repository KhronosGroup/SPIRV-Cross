static float4 gl_PositionIn[4];
static float4 gl_Position[4];
static uint gl_PrimitiveID;
static int gl_InvocationID;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
static int gl_PatchVerticesIn;
static float4 tPos[4];
static float4 vPos[4];
static float4 pColor;

struct SPIRV_Cross_Input
{
    float4 vPos : TEXCOORD0;
    float4 gl_PositionIn : SV_Position;
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[4] : SV_TessFactor;
    float gl_TessLevelInner[2] : SV_InsideTessFactor;
    float4 pColor : PATCH1;
};

struct SPIRV_Cross_Output
{
    float4 tPos : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void tesc_main()
{
    tPos[gl_InvocationID] = vPos[gl_InvocationID] * 2.0f;
    gl_Position[gl_InvocationID] = gl_PositionIn[gl_InvocationID];
    if (gl_InvocationID == 0)
    {
        float _71 = (1.0f + length(tPos[1] - tPos[0])) + length(tPos[3] - tPos[2]);
        gl_TessLevelOuter[0] = _71;
        gl_TessLevelOuter[1] = _71;
        gl_TessLevelOuter[2] = _71;
        gl_TessLevelOuter[3] = _71;
        gl_TessLevelInner[0] = _71;
        gl_TessLevelInner[1] = _71;
    }
    if (gl_InvocationID == 3)
    {
        pColor = float4(float(gl_PrimitiveID), float(gl_PatchVerticesIn), 0.0f, 1.0f);
    }
}

SPIRV_Cross_PatchConstant tesc_patch_constants(InputPatch<SPIRV_Cross_Input, 4> stage_input, const OutputPatch<SPIRV_Cross_Output, 4> stage_output, uint gl_PrimitiveIDIn : SV_PrimitiveID)
{
    gl_PrimitiveID = gl_PrimitiveIDIn;
    gl_PatchVerticesIn = 4;
    for (int i = 0; i < 4; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
        vPos[i] = stage_input[i].vPos;
    }
    for (int j = 0; j < 4; j++)
    {
        gl_Position[j] = stage_output[j].gl_Position;
        tPos[j] = stage_output[j].tPos;
    }
    for (int invocation = 0; invocation < 4; invocation++)
    {
        gl_InvocationID = invocation;
        tesc_main();
    }
    SPIRV_Cross_PatchConstant patch_output;
    patch_output.gl_TessLevelOuter[0] = gl_TessLevelOuter[0];
    patch_output.gl_TessLevelOuter[1] = gl_TessLevelOuter[1];
    patch_output.gl_TessLevelOuter[2] = gl_TessLevelOuter[2];
    patch_output.gl_TessLevelOuter[3] = gl_TessLevelOuter[3];
    patch_output.gl_TessLevelInner[0] = gl_TessLevelInner[0];
    patch_output.gl_TessLevelInner[1] = gl_TessLevelInner[1];
    patch_output.pColor = pColor;
    return patch_output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("tesc_patch_constants")]
SPIRV_Cross_Output main(InputPatch<SPIRV_Cross_Input, 4> stage_input, uint gl_InvocationIDIn : SV_OutputControlPointID, uint gl_PrimitiveIDIn : SV_PrimitiveID)
{
    gl_PrimitiveID = gl_PrimitiveIDIn;
    gl_PatchVerticesIn = 4;
    for (int i = 0; i < 4; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
        vPos[i] = stage_input[i].vPos;
    }
    gl_InvocationID = int(gl_InvocationIDIn);
    tesc_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position[gl_InvocationIDIn];
    stage_output.tPos = tPos[gl_InvocationIDIn];
    return stage_output;
}
