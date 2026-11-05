#version 450
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec2 vUV[];
layout(location = 0) out vec2 gUV;

void main()
{
	for (int i = 0; i < 3; i++)
	{
		gUV = vUV[i];
		gl_Position = gl_in[i].gl_Position + vec4(0.1, 0.0, 0.0, 0.0);
		EmitVertex();
	}
	EndPrimitive();
}
