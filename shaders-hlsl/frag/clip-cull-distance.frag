#version 450

in float gl_ClipDistance[2];
in float gl_CullDistance[2];

layout(location = 0) out float FragColor;

void main()
{
	FragColor = gl_ClipDistance[0] + gl_CullDistance[0] + gl_ClipDistance[1] + gl_CullDistance[1];
}

