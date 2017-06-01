#version 450

out gl_PerVertex
{
   vec4 gl_Position;
};

layout(std140) uniform UBO
{
    mat4 uMVP;
};
in vec4 aVertex;
in vec3 aNormal;
out vec3 vNormal;

void main()
{
    gl_Position = uMVP * aVertex;
    vNormal = aNormal;
}
