static const float2 _32[3] = { (-1.0f).xx, float2(1.0f, -1.0f), float2(0.0f, 1.0f) };

static float4 gl_Position;
static float2 gTexCoord;

struct SPIRV_Cross_Input
{
};

struct SPIRV_Cross_Output
{
    float2 gTexCoord : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void geom_main(point SPIRV_Cross_Input stage_input[1], inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    for (int i = 0; i < 3; i++)
    {
        gTexCoord = _32[i];
        gl_Position = float4(_32[i], 0.0f, 1.0f);
        {
            SPIRV_Cross_Output stage_output;
            stage_output.gl_Position = gl_Position;
            stage_output.gTexCoord = gTexCoord;
            geometry_stream.Append(stage_output);
        }
    }
    geometry_stream.RestartStrip();
}

[maxvertexcount(3)]
void main(point SPIRV_Cross_Input stage_input[1], inout TriangleStream<SPIRV_Cross_Output> geometry_stream)
{
    geom_main(stage_input, geometry_stream);
}
