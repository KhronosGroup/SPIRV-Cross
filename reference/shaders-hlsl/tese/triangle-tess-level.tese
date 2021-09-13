static float4 gl_Position;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
static float3 gl_TessCoord;
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};
struct SPIRV_Cross_DS_Constant_Input
{
    float gl_TessLevelOuter[3] : SV_TessFactor;
    float gl_TessLevelInner : SV_InsideTessFactor;
    float3 gl_TessCoord : SV_DomainLocation;
};
void tese_main()
{
    gl_Position = float4((gl_TessCoord.x * gl_TessLevelInner[0]) * gl_TessLevelOuter[0], (gl_TessCoord.y * gl_TessLevelInner[0]) * gl_TessLevelOuter[1], (gl_TessCoord.z * gl_TessLevelInner[0]) * gl_TessLevelOuter[2], 1.0f);
}
[domain("tri")]
SPIRV_Cross_Output main(SPIRV_Cross_DS_Constant_Input stage_input)
{
    gl_TessLevelOuter[0] = stage_input.gl_TessLevelOuter[0];
    gl_TessLevelOuter[1] = stage_input.gl_TessLevelOuter[1];
    gl_TessLevelOuter[2] = stage_input.gl_TessLevelOuter[2];
    gl_TessLevelInner[0] = stage_input.gl_TessLevelInner;
    gl_TessCoord = stage_input.gl_TessCoord;
    tese_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}