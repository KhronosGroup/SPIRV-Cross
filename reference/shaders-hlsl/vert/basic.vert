struct _UBO
{
    float4x4 uMVP;
};

cbuffer UBO
{
    _UBO _16;
};
uniform float4 gl_HalfPixel;

static float4 gl_Position;
static float4 aVertex;
static float3 vNormal;
static float3 aNormal;

struct InputVert
{
    float3 aNormal : TEXCOORD0;
    float4 aVertex : TEXCOORD1;
};

struct OutputVert
{
    float3 vNormal : TEXCOORD0;
    float4 gl_Position : POSITION;
};

void vert_main()
{
    gl_Position = mul(aVertex, _16.uMVP);
    vNormal = aNormal;
}

OutputVert main(InputVert input)
{
    aVertex = input.aVertex;
    aNormal = input.aNormal;
    vert_main();
    OutputVert output;
    output.gl_Position = gl_Position;
    output.vNormal = vNormal;
    output.gl_Position.x = output.gl_Position.x - gl_HalfPixel.x * output.gl_Position.w;
    output.gl_Position.y = output.gl_Position.y + gl_HalfPixel.y * output.gl_Position.w;
    return output;
}
