static uint gl_InvocationID;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
static float3 vtxColor[3];

struct SPIRV_Cross_Input
{
};

struct SPIRV_Cross_Output
{
    float3 vtxColor : TEXCOORD0;
};

struct SPIRV_Cross_PatchConstant
{
    float gl_TessLevelOuter[3] : SV_TessFactor;
    float gl_TessLevelInner[1] : SV_InsideTessFactor;
};

void tesc_main()
{
    float _18 = float(gl_InvocationID);
    float3 _19 = _18.xxx;
    vtxColor[gl_InvocationID] = _19;
}

void tesc_main_patch()
{
    float _18 = float(gl_InvocationID);
    float3 _19 = _18.xxx;
    vtxColor[gl_InvocationID] = _19;
    gl_TessLevelOuter[0] = 1.0f;
    gl_TessLevelOuter[1] = 2.0f;
    gl_TessLevelOuter[2] = 3.0f;
    gl_TessLevelInner[0] = 4.0f;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patch_constant")]
SPIRV_Cross_Output main(InputPatch<SPIRV_Cross_Input, 3> patch, uint uCPID : SV_OutputControlPointID)
{
    gl_InvocationID = uCPID;
    tesc_main();
    SPIRV_Cross_Output stage_output;
    stage_output.vtxColor = vtxColor[gl_InvocationID];
    return stage_output;
}

SPIRV_Cross_PatchConstant patch_constant(InputPatch<SPIRV_Cross_Input, 3> patch)
{
    gl_InvocationID = 0u;
    tesc_main_patch();
    SPIRV_Cross_PatchConstant patch_output;
    patch_output.gl_TessLevelOuter[0] = gl_TessLevelOuter[0];
    patch_output.gl_TessLevelOuter[1] = gl_TessLevelOuter[1];
    patch_output.gl_TessLevelOuter[2] = gl_TessLevelOuter[2];
    patch_output.gl_TessLevelInner[0] = gl_TessLevelInner[0];
    return patch_output;
}
