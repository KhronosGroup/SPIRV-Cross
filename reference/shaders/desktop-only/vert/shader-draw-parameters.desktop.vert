#version 460

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = vec4(float(gl_BaseVertex), float(gl_BaseInstance), float(gl_DrawID), 1);
}
