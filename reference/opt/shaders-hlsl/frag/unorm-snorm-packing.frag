static float4 FP32Out;
static uint UNORM8;
static uint SNORM8;
static uint UNORM8Out;
static float4 FP32;
static uint SNORM8Out;

struct SPIRV_Cross_Input
{
    nointerpolation uint SNORM8 : TEXCOORD0;
    nointerpolation uint UNORM8 : TEXCOORD1;
    nointerpolation float4 FP32 : TEXCOORD2;
};

struct SPIRV_Cross_Output
{
    float4 FP32Out : SV_Target0;
    uint UNORM8Out : SV_Target1;
    uint SNORM8Out : SV_Target2;
};

uint SPIRV_Cross_packUnorm4x8(float4 value)
{
    uint4 Packed = uint4(round(saturate(value) * 255.0));
    return Packed.x | (Packed.y << 8) | (Packed.z << 16) | (Packed.w << 24);
}

float4 SPIRV_Cross_unpackUnorm4x8(uint value)
{
    uint4 Packed = uint4(value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff, value >> 24);
    return float4(Packed) / 255.0;
}

uint SPIRV_Cross_packSnorm4x8(float4 value)
{
    int4 Packed = int4(round(clamp(value, -1.0, 1.0) * 127.0)) & 0xff;
    return uint(Packed.x | (Packed.y << 8) | (Packed.z << 16) | (Packed.w << 24));
}

float4 SPIRV_Cross_unpackSnorm4x8(uint value)
{
    int SignedValue = int(value);
    int4 Packed = int4(SignedValue << 24, SignedValue << 16, SignedValue << 8, SignedValue) >> 24;
    return clamp(float4(Packed) / 127.0, -1.0, 1.0);
}

void frag_main()
{
    FP32Out = SPIRV_Cross_unpackUnorm4x8(UNORM8);
    FP32Out = SPIRV_Cross_unpackSnorm4x8(SNORM8);
    UNORM8Out = SPIRV_Cross_packUnorm4x8(FP32);
    SNORM8Out = SPIRV_Cross_packSnorm4x8(FP32);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    UNORM8 = stage_input.UNORM8;
    SNORM8 = stage_input.SNORM8;
    FP32 = stage_input.FP32;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FP32Out = FP32Out;
    stage_output.UNORM8Out = UNORM8Out;
    stage_output.SNORM8Out = SNORM8Out;
    return stage_output;
}
