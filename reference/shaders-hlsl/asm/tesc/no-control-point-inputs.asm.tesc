static float4 gl_Position[3];
static int gl_InvocationID;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
struct SPIRV_Cross_Input
{
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[3] : SV_TessFactor;
    float gl_TessLevelInner : SV_InsideTessFactor;
};

struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};

void tesc_main()
{
    gl_Position[gl_InvocationID] = float4(float(gl_InvocationID), 0.0f, 0.0f, 1.0f);
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 2.0f;
        gl_TessLevelOuter[1] = 2.0f;
        gl_TessLevelOuter[2] = 2.0f;
        gl_TessLevelInner[0] = 2.0f;
    }
}

SPIRV_Cross_PatchConstant tesc_patch_constants(InputPatch<SPIRV_Cross_Input, 3> stage_input, const OutputPatch<SPIRV_Cross_Output, 3> stage_output)
{
    for (int j = 0; j < 3; j++)
    {
        gl_Position[j] = stage_output[j].gl_Position;
    }
    for (int invocation = 0; invocation < 3; invocation++)
    {
        gl_InvocationID = invocation;
        tesc_main();
    }
    SPIRV_Cross_PatchConstant patch_output;
    patch_output.gl_TessLevelOuter[0] = gl_TessLevelOuter[0];
    patch_output.gl_TessLevelOuter[1] = gl_TessLevelOuter[1];
    patch_output.gl_TessLevelOuter[2] = gl_TessLevelOuter[2];
    patch_output.gl_TessLevelInner = gl_TessLevelInner[0];
    return patch_output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(3)]
[patchconstantfunc("tesc_patch_constants")]
SPIRV_Cross_Output main(InputPatch<SPIRV_Cross_Input, 3> stage_input, uint gl_InvocationIDIn : SV_OutputControlPointID)
{
    gl_InvocationID = int(gl_InvocationIDIn);
    tesc_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position[gl_InvocationIDIn];
    return stage_output;
}
