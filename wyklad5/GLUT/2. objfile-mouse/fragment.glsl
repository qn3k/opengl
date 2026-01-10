#version 150 core

// Zmienne wejsciowe
in vec4 inoutPos;
in vec3 inoutColor;

// Zmienna wyjsciowa
out vec4 outColor;


void main()
{

	// odleglosc od poczatku ukladu wspolrzednych
    float odl = sqrt(inoutPos.x*inoutPos.x + inoutPos.y*inoutPos.y + inoutPos.z*inoutPos.z);

    // szalone kolory
    vec3 crazyColor = vec3(
		sin(odl*3.0f)/2.0f + 0.3f,
		sin(inoutPos.y)/2.0f + 0.3f,
		sin(gl_PrimitiveID/100.0)/5.0f + 0.3f
	);



	vec3 myColor = crazyColor + inoutColor.x/2.0;

	outColor = vec4(myColor, 1.0);
}
