#version 310 es

uniform vec4 UBO[8];
out vec4 oA;
out vec4 oB;
out vec4 oC;
out vec4 oD;
out vec4 oE;
out vec4 oF;

void main()
{
    gl_Position = vec4(0.0);
    oA = UBO[0];
    oB = vec4(UBO[1].xy, UBO[1].zw);
    oC = vec4(UBO[2].x, UBO[3].xyz);
    oD = vec4(UBO[4].xyz, UBO[4].w);
    oE = vec4(UBO[5].x, UBO[5].y, UBO[5].z, UBO[5].w);
    oF = vec4(UBO[6].x, UBO[6].zw, UBO[7].x);
}

