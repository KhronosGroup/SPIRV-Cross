#version 450

layout (set = 0, binding = 0) uniform vertexUniformBuffer1 {
    mat2 transform1;
};

layout (set = 1, binding = 0) uniform vertexUniformBuffer2{
    mat2 transform2;
};

void main() {
    const vec2 pos[3] = vec2[3](vec2(-1.f, -1.f), vec2(1.f, -1.f), vec2(-1.f, 1.f));
    gl_Position = vec4((transform1 + transform2) * pos[0], 0.f, 1.f);
}

