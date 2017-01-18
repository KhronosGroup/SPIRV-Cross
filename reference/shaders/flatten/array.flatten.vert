#version 310 es

uniform vec4 UBO[14];
in vec4 aVertex;

void main()
{
    gl_Position = (mat4(UBO[0], UBO[1], UBO[2], UBO[3]) * aVertex) + UBO[13];
}

