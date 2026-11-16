#version 420
#extension GL_ARB_shader_storage_buffer_object : require

layout(std430, binding = 0) buffer SSBO
{
	vec4 colors[];
};

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = colors[0];
}
