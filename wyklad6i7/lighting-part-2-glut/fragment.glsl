#version 150 core

// Kolor ostateczny
out vec4 outColor;

// Dane wejsciowe
in vec4 Position;
in vec2 UV;
in vec3 Normal;
in vec3 lightCoef;

uniform vec3 cameraPos;

void main()
{
	// odleglosc od poczatku ukladu wspolrzednych
	float dist = length(Position);

	// szalone kolory
	vec3 crazyColor = vec3(
		sin(dist*3.0)/2.0 + 0.5,
		sin(Position.y)/2.0 + 0.5,
		0.5
	);

	// Kolor poczatkowy - tutaj mozna dodac kolor
	// z tekstury
	vec3 fragColor = crazyColor;

	// obilczanie ostatecznego koloru
	//lightCoef *
	vec3 finalColor = fragColor;


	outColor = vec4(lightCoef * finalColor, 1.0);
}
