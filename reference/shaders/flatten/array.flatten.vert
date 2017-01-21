#version 310 es

uniform vec4 UBO[16];
in vec4 aVertex;

void main()
{
    vec4 offset = (UBO[10] + UBO[5]) + vec4(UBO[14].x);
    gl_Position = ((mat4(UBO[0], UBO[1], UBO[2], UBO[3]) * aVertex) + UBO[15]) + offset;
}

