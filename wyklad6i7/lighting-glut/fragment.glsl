#version 150 core

// Kolor ostateczny
out vec4 outColor;

// Dane wejsciowe z shadera wierzcholkow
in vec3 fragPos;
in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragLightCoef;


void main()
{
	// odleglosc od poczatku ukladu wspolrzednych
	float dist = length(fragPos);

	// szalone kolory
	vec3 crazyColor = vec3(
		sin(dist*3.0)/2.0 + 0.5,
		sin(fragPos.y)/2.0 + 0.5,
		0.5
	);

	// Kolor poczatkowy - tutaj mozna dodac kolor
	// z tekstury
	vec3 fragColor = crazyColor;


	outColor = vec4(fragLightCoef * fragColor, 1.0);
}
