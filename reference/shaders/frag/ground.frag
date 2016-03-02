#version 310 es
precision mediump float;
precision highp int;

layout(binding = 4, std140) uniform GlobalPSData
{
    vec4 g_CamPos;
    vec4 g_SunDir;
    vec4 g_SunColor;
    vec4 g_ResolutionParams;
    vec4 g_TimeParams;
    vec4 g_FogColor_Distance;
} _56;

layout(binding = 2) uniform mediump sampler2D TexNormalmap;

layout(location = 3) out vec4 LightingOut;
layout(location = 2) out vec4 NormalOut;
layout(location = 1) out vec4 SpecularOut;
layout(location = 0) out vec4 AlbedoOut;
layout(location = 0) in vec2 TexCoord;
layout(location = 1) in vec3 EyeVec;

float saturate(float x)
{
    return clamp(x, 0.000000, 1.000000);
}

void Resolve(vec3 Albedo, vec3 Normal, float Roughness, float Metallic)
{
    LightingOut = vec4(0.000000);
    NormalOut = vec4(((Normal * 0.500000) + vec3(0.500000)), 0.000000);
    SpecularOut = vec4(Roughness, Metallic, 0.000000, 0.000000);
    AlbedoOut = vec4(Albedo, 1.000000);
}

void main()
{
    vec3 Normal = ((texture(TexNormalmap, TexCoord).xyz * 2.000000) - vec3(1.000000));
    Normal = normalize(Normal);
    highp float param = (length(EyeVec) / 1000.000000);
    vec2 scatter_uv;
    scatter_uv.x = saturate(param);
    vec3 nEye = normalize(EyeVec);
    scatter_uv.y = 0.000000;
    vec3 Color = vec3(0.100000, 0.300000, 0.100000);
    vec3 grass = vec3(0.100000, 0.300000, 0.100000);
    vec3 dirt = vec3(0.100000);
    vec3 snow = vec3(0.800000);
    float grass_snow = smoothstep(0.000000, 0.150000, ((_56.g_CamPos.y + EyeVec.y) / 200.000000));
    vec3 base = mix(grass, snow, vec3(grass_snow));
    float edge = smoothstep(0.700000, 0.750000, Normal.y);
    Color = mix(dirt, base, vec3(edge));
    Color = (Color * Color);
    float Roughness = (1.000000 - (edge * grass_snow));
    highp vec3 param_1 = Color;
    highp vec3 param_2 = Normal;
    highp float param_3 = Roughness;
    highp float param_4 = 0.000000;
    Resolve(param_1, param_2, param_3, param_4);
}

