static float4 gl_Position;
static float gl_TessLevelOuter[4];
static float gl_TessLevelInner[2];
static float3 gl_TessCoord;
struct SPIRV_Cross_DS_Point_Input
{
    float4 Floats : TEXCOORD0;
    float4 Floats2 : TEXCOORD2;
};
struct SPIRV_Cross_Output
{
    float4 gl_Position : SV_Position;
};
struct SPIRV_Cross_DS_Constant_Input
{
    float gl_TessLevelOuter[4] : SV_TessFactor;
    float gl_TessLevelInner[2] : SV_InsideTessFactor;
    float2 gl_TessCoord : SV_DomainLocation;
};
void set_position(OutputPatch<SPIRV_Cross_DS_Point_Input,32> gl_in)
{
    gl_Position = (gl_in[0].Floats * gl_TessCoord.x) + (gl_in[1].Floats2 * gl_TessCoord.y);
}
void tese_main(OutputPatch<SPIRV_Cross_DS_Point_Input,32> gl_in)
{
    set_position(gl_in);
}
[domain("quad")]
SPIRV_Cross_Output main(SPIRV_Cross_DS_Constant_Input stage_input, OutputPatch<SPIRV_Cross_DS_Point_Input, 32> gl_in)
{
    gl_TessLevelOuter = stage_input.gl_TessLevelOuter;
    gl_TessLevelInner = stage_input.gl_TessLevelInner;
    gl_TessCoord = float3(stage_input.gl_TessCoord, 0.0);
    tese_main(gl_in);
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    return stage_output;
}