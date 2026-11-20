#version 450
#extension GL_EXT_fragment_shader_barycentric : require

layout(location = 0) pervertexEXT in vec4 vertex_color[];
layout(location = 0) out vec4 color;

void main()
{
    color = gl_BaryCoordEXT.x * vertex_color[0] +
            gl_BaryCoordEXT.y * vertex_color[1] +
            gl_BaryCoordEXT.z * vertex_color[2];
}
