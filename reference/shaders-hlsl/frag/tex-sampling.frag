Texture2D<float4> tex;
SamplerState _tex_sampler;

static float2 texCoord;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float2 texCoord : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float4 FragColor : SV_Target0;
};

float SPIRV_Cross_projectTextureCoordinate(float2 coord)
{
    return coord.x / coord.y;
}

float2 SPIRV_Cross_projectTextureCoordinate(float3 coord)
{
    return float2(coord.x, coord.y) / coord.z;
}

float3 SPIRV_Cross_projectTextureCoordinate(float4 coord)
{
    return float3(coord.x, coord.y, coord.z) / coord.w;
}

void frag_main()
{
    float4 texcolor = tex.Sample(_tex_sampler, texCoord);
    texcolor += tex.Sample(_tex_sampler, texCoord, int2(1, 2));
    texcolor += tex.SampleLevel(_tex_sampler, texCoord, 2.0f);
    texcolor += tex.SampleGrad(_tex_sampler, texCoord, float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    texcolor += tex.Sample(_tex_sampler, SPIRV_Cross_projectTextureCoordinate(float3(texCoord, 2.0f)));
    texcolor += tex.SampleBias(_tex_sampler, texCoord, 1.0f);
    FragColor = texcolor;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    texCoord = stage_input.texCoord;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
