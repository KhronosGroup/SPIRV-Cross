#version 450
layout(vertices = 3) out;

struct VertexOutput
{
    vec4 pos;
    vec2 uv;
};

struct VertexOutput_1
{
    vec2 uv;
};

layout(location = 0) in VertexOutput_1 p[];
layout(location = 0) out VertexOutput_1 _entryPointOutput[3];

void main()
{
    VertexOutput param[3] = VertexOutput[](VertexOutput(gl_in[0].gl_Position, p[0].uv), VertexOutput(gl_in[1].gl_Position, p[1].uv), VertexOutput(gl_in[2].gl_Position, p[2].uv));
    gl_out[gl_InvocationID].gl_Position = param[gl_InvocationID].pos;
    _entryPointOutput[gl_InvocationID].uv = param[gl_InvocationID].uv;
    barrier();
    if (int(gl_InvocationID) == 0)
    {
        vec2 _174 = vec2(1.0) + p[0].uv;
        float _175 = _174.x;
        gl_TessLevelOuter[0] = _175;
        gl_TessLevelOuter[1] = _175;
        gl_TessLevelOuter[2] = _175;
        gl_TessLevelInner[0] = _175;
    }
}

