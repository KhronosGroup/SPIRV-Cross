#version 460
#extension GL_EXT_mesh_shader : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) flat in int64_t scalar_i64;
layout(location = 0, component = 2) flat in uint64_t scalar_u64;
layout(location = 1) flat in i64vec2 vector_i64_2;
layout(location = 2) flat in u64vec3 vector_u64_3;
layout(location = 4) perprimitiveEXT flat in i64vec4 primitive_i64_4;

layout(location = 0) out vec4 color;

void main()
{
    bool valid = scalar_i64 == int64_t(-1) && scalar_u64 == uint64_t(2) &&
                 vector_i64_2 == i64vec2(-3, 4) && vector_u64_3 == u64vec3(5, 6, 7) &&
                 primitive_i64_4 == i64vec4(-8, 9, -10, 11);
    color = valid ? vec4(1.0) : vec4(0.0);
}
