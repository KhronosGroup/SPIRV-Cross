static float4 gl_PositionIn[2];
static float4 gl_Position[2];
static int gl_InvocationID;
static float gl_TessLevelOuter[4];
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

void tesc_main()
{
    gl_Position[gl_InvocationID] = gl_PositionIn[gl_InvocationID];
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 1.0f;
        gl_TessLevelOuter[1] = 8.0f;
    }
}

SPIRV_Cross_PatchConstant tesc_patch_constants(InputPatch<SPIRV_Cross_Input, 2> stage_input, const OutputPatch<SPIRV_Cross_Output, 2> stage_output)
{
    for (int i = 0; i < 2; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
    }
    for (int j = 0; j < 2; j++)
    {
        gl_Position[j] = stage_output[j].gl_Position;
    }
    for (int invocation = 0; invocation < 2; invocation++)
    {
        gl_InvocationID = invocation;
        tesc_main();
    }
    SPIRV_Cross_PatchConstant patch_output;
    patch_output.gl_TessLevelOuter[0] = gl_TessLevelOuter[0];
    patch_output.gl_TessLevelOuter[1] = gl_TessLevelOuter[1];
    return patch_output;
}

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(2)]
[patchconstantfunc("tesc_patch_constants")]
SPIRV_Cross_Output main(InputPatch<SPIRV_Cross_Input, 2> stage_input, uint gl_InvocationIDIn : SV_OutputControlPointID)
{
    for (int i = 0; i < 2; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
    }
    gl_InvocationID = int(gl_InvocationIDIn);
    tesc_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position[gl_InvocationIDIn];
    return stage_output;
}
