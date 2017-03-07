static float4 gl_FragCoord;
static float gl_FragDepth;
static float4 FragColor;
static float4 vColor;

struct SPIRV_Cross_Input
{
    float4 gl_FragCoord : VPOS;
    float4 vColor : TEXCOORD0;
};

struct SPIRV_Cross_Output
{
    float gl_FragDepth : DEPTH;
    float4 FragColor : COLOR0;
};

void frag_main()
{
    FragColor = gl_FragCoord + vColor;
    gl_FragDepth = 0.5f;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    gl_FragCoord = stage_input.gl_FragCoord + float4(0.5f, 0.5f, 0.0f, 0.0f);
    vColor = stage_input.vColor;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_FragDepth = gl_FragDepth;
    stage_output.FragColor = FragColor;
    return stage_output;
}
