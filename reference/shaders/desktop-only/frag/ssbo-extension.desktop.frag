#version 420
#extension GL_ARB_shader_storage_buffer_object : require

layout(binding = 0, std430) buffer SSBO
{
    vec4 colors[];
} _13;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = _13.colors[0];
}

