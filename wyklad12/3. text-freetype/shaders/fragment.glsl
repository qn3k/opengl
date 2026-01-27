#version 150 core

in vec4 fragPos;
out vec4 outColor;


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

	outColor = vec4(crazyColor, 1.0);

}
