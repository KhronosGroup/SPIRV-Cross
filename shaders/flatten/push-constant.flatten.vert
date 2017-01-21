#version 310 es

layout(push_constant, std430) uniform PushMe
{
   mat4 MVP;
   mat2 Rot; // The MatrixStride will be 8 here.
} registers;

layout(location = 0) in vec2 Rot;
layout(location = 1) in vec4 Pos;
layout(location = 0) out vec2 vRot;
void main()
{
   gl_Position = registers.MVP * Pos;
   vRot = registers.Rot * Rot;
}
