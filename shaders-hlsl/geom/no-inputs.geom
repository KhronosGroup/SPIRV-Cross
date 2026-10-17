#version 450

// A geometry shader that reads no input: its HLSL signature still takes the input array
layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) out vec2 gTexCoord;

const vec2 corners[3] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(0.0, 1.0));

void main()
{
    for (int i = 0; i < 3; i++)
    {
        gTexCoord = corners[i];
        gl_Position = vec4(corners[i], 0.0, 1.0);
        EmitVertex();
    }
    EndPrimitive();
}
