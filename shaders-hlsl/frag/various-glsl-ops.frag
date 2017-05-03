#version 450

in vec2 interpolant;

out vec4 FragColor;

void main()
{
	vec4 color = vec4(0.0, 0.0, 0.0, interpolateAtOffset(interpolant, vec2(0.1, 0.1)));

	// glslang's HLSL parser currently fails here
	//color += vec4(0.0, 0.0, 0.0, interpolateAtSample(interpolant, gl_SampleID));
	//color += vec4(0.0, 0.0, 0.0, interpolateAtCentroid(interpolant));

	color += vec4(0.0, 0.0, 0.0, dFdxCoarse(interpolant.x));
	FragColor = color;
}
