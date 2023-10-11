#version 310 es
#extension GL_EXT_geometry_shader : require

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

layout(location = 0) in VertexData {
    vec3 normal;
    vec4 position;
} vin[];

layout(location = 0) out vec3 vNormal;

void main()
{
    gl_Position = vin[0].position;
    vNormal = vin[0].normal;
    EmitVertex();

    gl_Position = vin[1].position;
    vNormal = vin[1].normal;
    EmitVertex();

    EndPrimitive();
}
