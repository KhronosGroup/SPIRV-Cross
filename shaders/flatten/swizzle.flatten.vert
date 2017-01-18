#version 310 es

// comments note the 16b alignment boundaries (see GL spec 7.6.2.2 Standard Uniform Block Layout)
layout(std140) uniform UBO
{
    // 16b boundary
    uniform vec4 A;
    // 16b boundary
    uniform vec2 B0;
    uniform vec2 B1;
    // 16b boundary
    uniform float C0;
    // 16b boundary (vec3 is aligned to 16b)
    uniform vec3 C1;
    // 16b boundary
    uniform vec3 D0;
    uniform float D1;
    // 16b boundary
    uniform float E0;
    uniform float E1;
    uniform float E2;
    uniform float E3;
    // 16b boundary
    uniform float F0;
    uniform vec2 F1;
    // 16b boundary (vec2 before us is aligned to 8b)
    uniform float F2;
};

out vec4 oA, oB, oC, oD, oE, oF;

void main()
{
    gl_Position = vec4(0.0);

    oA = A;
    oB = vec4(B0, B1);
    oC = vec4(C0, C1);
    oD = vec4(D0, D1);
    oE = vec4(E0, E1, E2, E3);
    oF = vec4(F0, F1, F2);
}
