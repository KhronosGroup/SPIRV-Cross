#version 450
#extension GL_ARB_shader_draw_parameters : enable

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = vec4(float(gl_BaseVertexARB), float(gl_BaseInstanceARB), float(gl_DrawIDARB), 1);
}
