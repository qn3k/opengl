// Vertex shader w wersji 3.3
// oraz profilu core (zamiast compatibility)
#version 330 core

// Dane pobrane z tablicy wierzcholkow VAO
// spod atrybutu o numerze 0 (ten sam
// podczas konfigurowania VAO
layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inColor;


// Shader wierzcholkow ma rowniez swoje
// predefiniowane dane wejsciowe

// nie odkomentowujemy
/*
in int gl_VertexID;
in int gl_InstanceID;
*/

// Predefiniowane dane wyjsciowe
// przekazane do kolejnego etapu potoku

// rowniez nie odkomentowujemy
// out vec4 gl_Position;

// Ustawienie zmiennej typu out, ktora zostanie
// przekazana do vertex shadera
out vec3 vColor;

void main()
{

	// zmienna typu vec4
	vec4 finalPosition = inPosition;

	// Ustalenie wartosci zmiennej out
	vColor = inColor;

	// finalna pozycja wierzcholka
	gl_Position = finalPosition;
}
