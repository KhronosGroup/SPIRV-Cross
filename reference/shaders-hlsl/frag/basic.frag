struct SPIRV_Cross_Texture2D_float4_SamplerState
{
    Texture2D<float4> img;
    SamplerState smpl;
};

SPIRV_Cross_Texture2D_float4_SamplerState uTex;

static float4 FragColor;
static float4 vColor;
static float2 vTex;

struct SPIRV_Cross_Input
{
    float4 vColor : TEXCOORD0;
    float2 vTex : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = vColor * uTex.img.Sample(uTex.smpl, vTex);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    vColor = stage_input.vColor;
    vTex = stage_input.vTex;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
