cbuffer Params : register(b0)
{
    float u_level : packoffset(c0);
};


static float4 gl_PositionIn[3];
static float4 gl_Position[3];
static int gl_InvocationID;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
static float2 tUV[3];
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
    float2 tUV : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void tesc_main()
{
    gl_Position[gl_InvocationID] = gl_PositionIn[gl_InvocationID];
    tUV[gl_InvocationID] = vUV[gl_InvocationID];
    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = u_level;
        gl_TessLevelOuter[1] = u_level;
        gl_TessLevelOuter[2] = u_level;
        gl_TessLevelInner[0] = u_level;
    }
}

SPIRV_Cross_PatchConstant tesc_patch_constants(InputPatch<SPIRV_Cross_Input, 3> stage_input, const OutputPatch<SPIRV_Cross_Output, 3> stage_output)
{
    for (int i = 0; i < 3; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
        vUV[i] = stage_input[i].vUV;
    }
    for (int j = 0; j < 3; j++)
    {
        gl_Position[j] = stage_output[j].gl_Position;
        tUV[j] = stage_output[j].tUV;
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
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("tesc_patch_constants")]
SPIRV_Cross_Output main(InputPatch<SPIRV_Cross_Input, 3> stage_input, uint gl_InvocationIDIn : SV_OutputControlPointID)
{
    for (int i = 0; i < 3; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
        vUV[i] = stage_input[i].vUV;
    }
    gl_InvocationID = int(gl_InvocationIDIn);
    tesc_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position[gl_InvocationIDIn];
    stage_output.tUV = tUV[gl_InvocationIDIn];
    return stage_output;
}
