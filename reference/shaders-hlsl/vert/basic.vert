struct _UBO
{
    float4x4 uMVP;
};

cbuffer UBO
{
    _UBO _16;
};

static float4 gl_Position;
static float4 aVertex;
static float3 vNormal;
static float3 aNormal;

struct SPIRV_Cross_Input
{
    float3 aNormal : TEXCOORD0;
    float4 aVertex : TEXCOORD1;
};

struct SPIRV_Cross_Output
{
    float3 vNormal : TEXCOORD0;
    float4 gl_Position : SV_Position;
};

void vert_main()
{
    gl_Position = mul(aVertex, _16.uMVP);
    vNormal = aNormal;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    aVertex = stage_input.aVertex;
    aNormal = stage_input.aNormal;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.vNormal = vNormal;
    return stage_output;
}
