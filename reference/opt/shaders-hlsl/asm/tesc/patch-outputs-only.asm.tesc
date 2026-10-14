static int gl_InvocationID;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
static float4 pCenter;
static float4 vPos[3];

struct SPIRV_Cross_Input
{
    float4 vPos : TEXCOORD0;
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[3] : SV_TessFactor;
    float gl_TessLevelInner : SV_InsideTessFactor;
    float4 pCenter : PATCH0;
};

struct SPIRV_Cross_Output
{
};

void tesc_main()
{
    if (gl_InvocationID == 0)
    {
        pCenter = ((vPos[0] + vPos[1]) + vPos[2]) * 0.3333333432674407958984375f.xxxx;
        gl_TessLevelOuter[0] = 3.0f;
        gl_TessLevelOuter[1] = 3.0f;
        gl_TessLevelOuter[2] = 3.0f;
        gl_TessLevelInner[0] = 3.0f;
    }
}

SPIRV_Cross_PatchConstant tesc_patch_constants(InputPatch<SPIRV_Cross_Input, 3> stage_input)
{
    for (int i = 0; i < 3; i++)
    {
        vPos[i] = stage_input[i].vPos;
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
    patch_output.pCenter = pCenter;
    return patch_output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(3)]
[patchconstantfunc("tesc_patch_constants")]
void main(InputPatch<SPIRV_Cross_Input, 3> stage_input, uint gl_InvocationIDIn : SV_OutputControlPointID)
{
    for (int i = 0; i < 3; i++)
    {
        vPos[i] = stage_input[i].vPos;
    }
    gl_InvocationID = int(gl_InvocationIDIn);
    tesc_main();
}
