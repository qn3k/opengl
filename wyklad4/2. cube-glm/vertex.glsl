// Vertex shader
#version 330 core

// Dane pobrane z VAO
layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inColor;

out vec3 fragColor;

// Tym razem macierze transformacji
// oraz rzutowania przesylamy z aplikacji
// wygenerowane tam za pomoca biblioteki GLM

uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;

void main()
{
	fragColor = inColor;

	// Nowe polozenie wierzcholkow jest wynikiem iloczynu 
	// pierwotnego polozenia oraz macierzy rzutowania i transformacji
	// Uwaga! Istotna jest kolejnosc mnozenia macierzy.
	
	vec4 newPosition = matProj * matView * matModel * inPosition;
	
	gl_Position = newPosition;
}
