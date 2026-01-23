#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords; // do tekstur
layout (location = 2) in vec3 aNormal;
layout (location = 3) in mat4 matModelInst; // pobierane z bufora instancji

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;
uniform bool bIsInstanced; // czy jest instancjonowany 

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main() {
    // 1. Wybór macierzy modelu
    mat4 finalModel = bIsInstanced ? matModelInst : matModel;

    // 2. Obliczenie pozycji w świecie (do oświetlenia w fragment shaderze)
    vec4 worldPos = finalModel * vec4(aPos, 1.0);
    FragPos = vec4(worldPos).xyz; 

    // 3. Obliczenie ostatecznej pozycji na ekranie
    // Mnożymy macierze rzutowania i widoku przez pozycję w świecie
    gl_Position = matProj * matView * worldPos;

    // 4. Przekazanie reszty danych
    Normal = mat3(transpose(inverse(finalModel))) * aNormal;  
    TexCoords = aTexCoords;
}