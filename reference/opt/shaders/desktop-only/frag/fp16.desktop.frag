#version 450
#if defined(GL_AMD_gpu_shader_half_float)
#extension GL_AMD_gpu_shader_half_float : require
#elif defined(GL_NV_gpu_shader5)
#extension GL_NV_gpu_shader5 : require
#else
#error No extension available for FP16.
#endif

layout(location = 3) in f16vec4 v4;

void main()
{
    f16vec4 _505;
    f16vec4 _577 = modf(v4, _505);
}

