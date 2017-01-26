uniform sampler2D uTex;

static float4 FragColor;
static float4 vColor;
static float2 vTex;

struct InputFrag
{
    float4 vColor : TEXCOORD0;
    float2 vTex : TEXCOORD1;
};

struct OutputFrag
{
    float4 FragColor : COLOR;
};

void frag_main()
{
    FragColor = vColor * tex2D(uTex, vTex);
}

OutputFrag main(InputFrag input)
{
    vColor = input.vColor;
    vTex = input.vTex;
    frag_main();
    OutputFrag output;
    output.FragColor = FragColor;
    return output;
}
