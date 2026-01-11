#version 150 core

// -----------------------------------
// NOWE: odbieramy zmienne bloku
// interfejsu
// -----------------------------------
in VS_OUT
{
	vec3 fragPos;
	vec2 fragUV;
	vec3 fragNormal;
} fs;

// Kolor ostateczny
out vec4 outColor;

// Zmienne jednorodne
uniform vec3 cameraPos;
uniform sampler2D tex0;


void main()
{
	vec3 fragColor = texture(tex0, fs.fragUV).rgb;

	outColor = vec4(fragColor, 1.0);
}
