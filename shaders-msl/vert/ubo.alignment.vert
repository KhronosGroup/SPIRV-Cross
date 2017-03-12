#version 310 es

layout(binding = 0, std140) uniform UBO
{
   mat4 mvp;
   vec2 targSize;
   vec3 color;      // vec3 following vec2 should cause MSL to add pad if float3 is packed
   float opacity;   // Single float following vec3 should cause MSL float3 to pack
};

in vec4 aVertex;
in vec3 aNormal;
out vec3 vNormal;
out vec3 vColor;
out vec2 vSize;

void main()
{
    gl_Position = mvp * aVertex;
    vNormal = aNormal;
    vColor = color * opacity;
    vSize = targSize * opacity;
}
