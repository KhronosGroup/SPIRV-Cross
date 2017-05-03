#version 450

uniform sampler1D tex1d;
uniform sampler2D tex2d;
uniform sampler3D tex3d;
uniform samplerCube texCube;

uniform sampler1DShadow tex1dShadow;
uniform sampler2DShadow tex2dShadow;
uniform samplerCubeShadow texCubeShadow;

uniform sampler1DArray tex1dArray;
uniform sampler2DArray tex2dArray;
uniform samplerCubeArray texCubeArray;

in float texCoord1d;
in vec2 texCoord2d;
in vec3 texCoord3d;
in vec4 texCoord4d;

out vec4 FragColor;

void main()
{
	vec4 texcolor = texture(tex1d, texCoord1d);
	texcolor += textureOffset(tex1d, texCoord1d, 1);
	texcolor += textureLod(tex1d, texCoord1d, 2);
	texcolor += textureGrad(tex1d, texCoord1d, 1.0, 2.0);
	texcolor += textureProj(tex1d, vec2(texCoord1d, 2.0));
	texcolor += texture(tex1d, texCoord1d, 1.0);

	texcolor += texture(tex2d, texCoord2d);
	texcolor += textureOffset(tex2d, texCoord2d, ivec2(1, 2));
	texcolor += textureLod(tex2d, texCoord2d, 2);
	texcolor += textureGrad(tex2d, texCoord2d, vec2(1.0, 2.0), vec2(3.0, 4.0));
	texcolor += textureProj(tex2d, vec3(texCoord2d, 2.0));
	texcolor += texture(tex2d, texCoord2d, 1.0);

	texcolor += texture(tex3d, texCoord3d);
	texcolor += textureOffset(tex3d, texCoord3d, ivec3(1, 2, 3));
	texcolor += textureLod(tex3d, texCoord3d, 2);
	texcolor += textureGrad(tex3d, texCoord3d, vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0));
	texcolor += textureProj(tex3d, vec4(texCoord3d, 2.0));
	texcolor += texture(tex3d, texCoord3d, 1.0);

	texcolor += texture(texCube, texCoord3d);
	texcolor += textureLod(texCube, texCoord3d, 2);
	texcolor += texture(texCube, texCoord3d, 1.0);

	texcolor.a += texture(tex1dShadow, vec3(texCoord1d, 0.0, 0.0));
	texcolor.a += texture(tex2dShadow, vec3(texCoord2d, 0.0));
	texcolor.a += texture(texCubeShadow, vec4(texCoord3d, 0.0));

	texcolor += texture(tex1dArray, texCoord2d);
	texcolor += texture(tex2dArray, texCoord3d);
	texcolor += texture(texCubeArray, texCoord4d);

	texcolor += textureGather(tex2d, texCoord2d);

	texcolor += texelFetch(tex2d, ivec2(1, 2), 0);

	FragColor = texcolor;
}
