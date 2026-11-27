static float4 gl_PositionIn[3];
static float4 gl_Position;
static float2 gUV;
static float2 vUV[3];

struct SPIRV_Cross_Input
{
    float2 vUV : TEXCOORD0;
    float4 gl_PositionIn : SV_Position;
};

struct SPIRV_Cross_Output
{
    float2 gUV : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void geom_main(triangle SPIRV_Cross_Input stage_input[3], inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    for (int _55 = 0; _55 < 3; )
    {
        gUV = vUV[_55];
        gl_Position = gl_PositionIn[_55] + float4(0.100000001490116119384765625f, 0.0f, 0.0f, 0.0f);
        {
            SPIRV_Cross_Output stage_output;
            stage_output.gl_Position = gl_Position;
            stage_output.gUV = gUV;
            geometry_stream.Append(stage_output);
        }
        _55++;
        continue;
    }
    geometry_stream.RestartStrip();
}

[maxvertexcount(3)]
void main(triangle SPIRV_Cross_Input stage_input[3], inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    for (int i = 0; i < 3; i++)
    {
        gl_PositionIn[i] = stage_input[i].gl_PositionIn;
        vUV[i] = stage_input[i].vUV;
    }
    geom_main(stage_input, geometry_stream);
}
