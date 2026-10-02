#version 450
#extension GL_EXT_multiview : require
layout(location = 0) out vec4 c;
void main() { gl_Position = vec4(float(gl_ViewIndex), float(gl_InstanceIndex), float(gl_VertexIndex), 1.0); c = vec4(float(gl_ViewIndex)); }
