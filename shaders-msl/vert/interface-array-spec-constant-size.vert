#version 450
layout(constant_id = 0) const int N = 2;
layout(location = 0) out vec4 v[N];
void main()
{
	gl_Position = vec4(1.0);
	for (int i = 0; i < N; i++)
		v[i] = vec4(float(i));
}
