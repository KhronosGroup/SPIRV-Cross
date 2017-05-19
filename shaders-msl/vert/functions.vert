#version 310 es

layout(std140) uniform UBO
{
    uniform mat4 uMVP;
    uniform vec3 rotDeg;
    uniform vec3 rotRad;
    uniform ivec2 bits;
};

layout(location = 0) in vec4 aVertex;
layout(location = 1) in vec3 aNormal;

out vec3 vNormal;
out vec3 vRotDeg;
out vec3 vRotRad;
out ivec2 vLSB;
out ivec2 vMSB;

void main()
{
    gl_Position = inverse(uMVP) * aVertex;
    vNormal = aNormal;
    vRotDeg = degrees(rotRad);
    vRotRad = radians(rotDeg);
    vLSB = findLSB(bits);
    vMSB = findMSB(bits);
}
