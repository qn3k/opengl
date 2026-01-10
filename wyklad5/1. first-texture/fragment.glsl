#version 150 core

// zmienne wejsciowe
in vec4 inoutPos;
in vec3 inoutColor;
in vec2 inoutUV;

// ---------------------------------
// NOWE: uchwyt tekstury jako zmienna
// jednorodna typu sampler2D
// ---------------------------------
uniform sampler2D tex;

// zmienna wyjsciowa
out vec4 outColor;


void main()
{
	// ---------------------------------
	// NOWE: pobranie wartosci tekstury 
	// dla zadanej wspolrzednej UV
	// ---------------------------------
	vec4 texColor = texture(tex, inoutUV);
	
	
	outColor = texColor;
}
