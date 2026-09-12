#version 450

layout(vertices = 3) out;

layout(location = 0) out vec3 vtxColor[];

void main()
{
    vtxColor[gl_InvocationID] = vec3(float(gl_InvocationID));

    barrier();

    if (gl_InvocationID == 0)
    {
        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = 2.0;
        gl_TessLevelOuter[2] = 3.0;
        gl_TessLevelInner[0] = 4.0;
    }
}
