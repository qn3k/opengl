#version 330 core

layout (location = 0) in vec3 aPos;   // Pozycja z pliku OBJ
layout (location = 2) in vec3 aNormal; // Normalna z pliku OBJ

out vec3 vLocalPos; // Wyjście: pozycja lokalna (do gradientu)
out vec3 vNormal;   // Wyjście: normalna (do oświetlenia, opcjonalnie)
out vec3 vFragPos;  // Wyjście: pozycja w świecie (do oświetlenia)

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;

void main()
{
    // Przekazujemy oryginalną pozycję wierzchołka 
    vLocalPos = aPos;
    
    // Obliczenia dla oświetlenia (standardowe)
    vNormal = mat3(transpose(inverse(matModel))) * aNormal;
    vFragPos = vec3(matModel * vec4(aPos, 1.0));

    gl_Position = matProj * matView * matModel * vec4(aPos, 1.0);
}