#version 460
#extension GL_EXT_ray_tracing : require

layout(set = 0, binding = 0) uniform sampler2D image;
layout(set = 0, binding = 1, std430) readonly buffer Data
{
    float values[];
};
layout(location = 0) rayPayloadInEXT vec4 payload;

void main()
{
    payload = texture(image, vec2(0.5)) + float(values.length());
}
