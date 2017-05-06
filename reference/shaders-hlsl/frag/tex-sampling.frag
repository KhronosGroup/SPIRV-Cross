struct SPIRV_Cross_Texture2DArray_float4_SamplerState
{
    Texture2DArray<float4> img;
    SamplerState smpl;
};

struct SPIRV_Cross_Texture1DArray_float4_SamplerState
{
    Texture1DArray<float4> img;
    SamplerState smpl;
};

struct SPIRV_Cross_Texture1D_float4_SamplerComparisonState
{
    Texture1D<float4> img;
    SamplerComparisonState smpl;
};

struct SPIRV_Cross_Texture1D_float4_SamplerState
{
    Texture1D<float4> img;
    SamplerState smpl;
};

struct SPIRV_Cross_Texture2D_float4_SamplerState
{
    Texture2D<float4> img;
    SamplerState smpl;
};

struct SPIRV_Cross_TextureCubeArray_float4_SamplerState
{
    TextureCubeArray<float4> img;
    SamplerState smpl;
};

struct SPIRV_Cross_Texture2D_float4_SamplerComparisonState
{
    Texture2D<float4> img;
    SamplerComparisonState smpl;
};

struct SPIRV_Cross_TextureCube_float4_SamplerState
{
    TextureCube<float4> img;
    SamplerState smpl;
};

struct SPIRV_Cross_TextureCube_float4_SamplerComparisonState
{
    TextureCube<float4> img;
    SamplerComparisonState smpl;
};

struct SPIRV_Cross_Texture3D_float4_SamplerState
{
    Texture3D<float4> img;
    SamplerState smpl;
};

SPIRV_Cross_Texture1D_float4_SamplerState tex1d;
SPIRV_Cross_Texture2D_float4_SamplerState tex2d;
SPIRV_Cross_Texture3D_float4_SamplerState tex3d;
SPIRV_Cross_TextureCube_float4_SamplerState texCube;
SPIRV_Cross_Texture1D_float4_SamplerComparisonState tex1dShadow;
SPIRV_Cross_Texture2D_float4_SamplerComparisonState tex2dShadow;
SPIRV_Cross_TextureCube_float4_SamplerComparisonState texCubeShadow;
SPIRV_Cross_Texture1DArray_float4_SamplerState tex1dArray;
SPIRV_Cross_Texture2DArray_float4_SamplerState tex2dArray;
SPIRV_Cross_TextureCubeArray_float4_SamplerState texCubeArray;

static float2 texCoord2d;
static float texCoord1d;
static float3 texCoord3d;
static float4 texCoord4d;
static float4 FragColor;

struct SPIRV_Cross_Input
{
    float texCoord1d : TEXCOORD0;
    float2 texCoord2d : TEXCOORD1;
    float3 texCoord3d : TEXCOORD2;
    float4 texCoord4d : TEXCOORD3;
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

float4 pass_to_function(SPIRV_Cross_Texture2D_float4_SamplerState t2d)
{
    return t2d.img.Sample(t2d.smpl, texCoord2d);
}

void frag_main()
{
    float4 texcolor = tex1d.img.Sample(tex1d.smpl, texCoord1d);
    texcolor += tex1d.img.Sample(tex1d.smpl, texCoord1d, 1);
    texcolor += tex1d.img.SampleLevel(tex1d.smpl, texCoord1d, 2.0f);
    texcolor += tex1d.img.SampleGrad(tex1d.smpl, texCoord1d, 1.0f, 2.0f);
    texcolor += tex1d.img.Sample(tex1d.smpl, SPIRV_Cross_projectTextureCoordinate(float2(texCoord1d, 2.0f)));
    texcolor += tex1d.img.SampleBias(tex1d.smpl, texCoord1d, 1.0f);
    texcolor += tex2d.img.Sample(tex2d.smpl, texCoord2d);
    texcolor += tex2d.img.Sample(tex2d.smpl, texCoord2d, int2(1, 2));
    texcolor += tex2d.img.SampleLevel(tex2d.smpl, texCoord2d, 2.0f);
    texcolor += tex2d.img.SampleGrad(tex2d.smpl, texCoord2d, float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    texcolor += tex2d.img.Sample(tex2d.smpl, SPIRV_Cross_projectTextureCoordinate(float3(texCoord2d, 2.0f)));
    texcolor += tex2d.img.SampleBias(tex2d.smpl, texCoord2d, 1.0f);
    texcolor += pass_to_function(tex2d);
    texcolor += tex3d.img.Sample(tex3d.smpl, texCoord3d);
    texcolor += tex3d.img.Sample(tex3d.smpl, texCoord3d, int3(1, 2, 3));
    texcolor += tex3d.img.SampleLevel(tex3d.smpl, texCoord3d, 2.0f);
    texcolor += tex3d.img.SampleGrad(tex3d.smpl, texCoord3d, float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
    texcolor += tex3d.img.Sample(tex3d.smpl, SPIRV_Cross_projectTextureCoordinate(float4(texCoord3d, 2.0f)));
    texcolor += tex3d.img.SampleBias(tex3d.smpl, texCoord3d, 1.0f);
    texcolor += texCube.img.Sample(texCube.smpl, texCoord3d);
    texcolor += texCube.img.SampleLevel(texCube.smpl, texCoord3d, 2.0f);
    texcolor += texCube.img.SampleBias(texCube.smpl, texCoord3d, 1.0f);
    float3 _182 = float3(texCoord1d, 0.0f, 0.0f);
    texcolor.w += tex1dShadow.img.SampleCmp(tex1dShadow.smpl, _182.x, _182.z);
    float3 _200 = float3(texCoord2d, 0.0f);
    texcolor.w += tex2dShadow.img.SampleCmp(tex2dShadow.smpl, _200.xy, _200.z);
    float4 _216 = float4(texCoord3d, 0.0f);
    texcolor.w += texCubeShadow.img.SampleCmp(texCubeShadow.smpl, _216.xyz, _216.w);
    texcolor += tex1dArray.img.Sample(tex1dArray.smpl, texCoord2d);
    texcolor += tex2dArray.img.Sample(tex2dArray.smpl, texCoord3d);
    texcolor += texCubeArray.img.Sample(texCubeArray.smpl, texCoord4d);
    texcolor += tex2d.img.Gather(tex2d.smpl, texCoord2d, 0);
    texcolor += tex2d.img.Load(int3(int2(1, 2), 0));
    FragColor = texcolor;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    texCoord2d = stage_input.texCoord2d;
    texCoord1d = stage_input.texCoord1d;
    texCoord3d = stage_input.texCoord3d;
    texCoord4d = stage_input.texCoord4d;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
