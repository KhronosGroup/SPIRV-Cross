#version 450

uniform sampler2D tex;
in vec2 texCoord;

out vec4 FragColor;

void main() {
	vec4 texcolor = texture(tex, texCoord);
	texcolor += textureOffset(tex, texCoord, ivec2(1, 2));
	texcolor += textureLod(tex, texCoord, 2);
	texcolor += textureGrad(tex, texCoord, vec2(1.0, 2.0), vec2(3.0, 4.0));
	texcolor += textureProj(tex, vec3(texCoord, 2.0));
	texcolor += texture(tex, texCoord, 1.0);
	FragColor = texcolor;
}
