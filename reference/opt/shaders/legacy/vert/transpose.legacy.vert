#version 100

struct Buffer
{
    mat4 MVPRowMajor;
    mat4 MVPColMajor;
    mat4 M;
};

uniform Buffer _13;

attribute vec4 Position;

mat4 SPIRV_Cross_workaround_load_row_major(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = (((SPIRV_Cross_workaround_load_row_major(_13.M) * (Position * _13.MVPRowMajor)) + (SPIRV_Cross_workaround_load_row_major(_13.M) * (SPIRV_Cross_workaround_load_row_major(_13.MVPColMajor) * Position))) + (SPIRV_Cross_workaround_load_row_major(_13.M) * (_13.MVPRowMajor * Position))) + (SPIRV_Cross_workaround_load_row_major(_13.M) * (Position * SPIRV_Cross_workaround_load_row_major(_13.MVPColMajor)));
}

