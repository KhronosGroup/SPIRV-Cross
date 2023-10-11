#version 310 es
#extension GL_EXT_geometry_shader : require

layout(triangles, invocations = 4) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in VertexData {
    vec3 normal;
    vec4 pos;
} vin[];

layout(location = 2) in vec4 pos[];

layout(location = 0) out vec3 vNormal;

void main()
{
    gl_Position = pos[0];
    vNormal = vin[0].normal;
    EmitVertex();

    gl_Position = pos[1];
    vNormal = vin[1].normal;
    EmitVertex();

    gl_Position = pos[2];
    vNormal = vin[2].normal;
    EmitVertex();

    EndPrimitive();
}
