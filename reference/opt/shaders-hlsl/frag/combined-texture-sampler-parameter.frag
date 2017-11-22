Texture2D<float4> uSampler : register(t1);
SamplerState _uSampler_sampler : register(s1);
Texture2D<float4> uSamplerShadow : register(t1);
SamplerComparisonState _uSamplerShadow_sampler : register(s1);

static float FragColor;

struct SPIRV_Cross_Output
{
    float FragColor : SV_Target0;
};

void frag_main()
{
    FragColor = (uSampler.Sample(_uSampler_sampler, float2(1.0f, 1.0f)) + uSampler.Load(int3(int2(10, 10), 0))).x + uSamplerShadow.SampleCmp(_uSamplerShadow_sampler, float3(1.0f, 1.0f, 1.0f).xy, 1.0f);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
