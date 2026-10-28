#version 450

layout(binding = 0) uniform sampler2D Samp;

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 a = texture(Samp, vUV);
    vec4 b = texture(Samp, vUV);
    FragColor = (gl_FragCoord.x < 100.0) ? a : b;
}

