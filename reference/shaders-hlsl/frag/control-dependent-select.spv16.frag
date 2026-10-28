Texture2D<float4> Samp : register(t0);
SamplerState _Samp_sampler : register(s0);

static float4 gl_FragCoord;
static float2 vUV;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float2 vUV : TEXCOORD0;
    float4 gl_FragCoord : SV_Position;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    float4 a = Samp.Sample(_Samp_sampler, vUV);
    float4 b = Samp.Sample(_Samp_sampler, vUV);
    FragColor = (gl_FragCoord.x < 100.0f) ? a : b;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord;
    gl_FragCoord.w = 1.0 / gl_FragCoord.w;
    vUV = stage_input.vUV;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
