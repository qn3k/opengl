#version 330 core
layout( location = 0 ) in vec4 inPosition;
layout( location = 1 ) in vec3 inNormal;

uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;

uniform mat4 lightProj;
uniform mat4 lightView;

out vec4 fragPos;
out vec4 fragPosLight;
out vec3 fragNormal;


void main()
{
	gl_Position = matProj * matView * matModel * inPosition;

	// Macierz normalna powinna byc wyliczona na CPU
	// Tutaj tylko dla uproszczenia przykladu
	mat3 matNormal = mat3(transpose(inverse(matModel)));
	fragNormal =  matNormal * inNormal;
	
	
	// Wspolrzedne wierzcholka w ukladzie sceny
	fragPos = matModel * inPosition;
	
	// Wspolrzedne wierzcholka w bryle obcinania kamery oswietlenia
	fragPosLight = lightProj * lightView * matModel * inPosition;
}
