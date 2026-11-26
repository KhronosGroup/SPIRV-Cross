#version 450

layout(binding = 0) uniform sampler2D Blah;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 vUV;

void main()
{
    FragColor = texture(Blah, vUV.xy);
}

