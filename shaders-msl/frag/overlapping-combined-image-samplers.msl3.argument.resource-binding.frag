#version 450

layout(set = 0, binding = 0) uniform sampler2D s0_2d;
layout(set = 0, binding = 0) uniform sampler2DShadow s0_2d_shadow;
layout(set = 0, binding = 0) uniform samplerCube s0_cube;

layout(location = 0) in vec3 vUV;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(s0_2d, vUV.xy) + texture(s0_2d_shadow, vUV) + texture(s0_cube, vUV);
}
