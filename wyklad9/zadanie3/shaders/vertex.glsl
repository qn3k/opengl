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
    // Wybieramy macierz: albo pojedynczą z uniform, albo z instancji
    mat4 finalModel = bIsInstanced ? matModelInst : matModel;

    vec4 worldPos = finalModel * vec4(aPos, 1.0);
    gl_Position = matProj * matView * worldPos;

    FragPos = vec3(finalModel * vec4(aPos, 1.0));
    // Przekazywanie normalnych (ważne dla oświetlenia)
    Normal = mat3(transpose(inverse(finalModel))) * aNormal;  
    TexCoords = aTexCoords;
    
    gl_Position = matProj * matView * vec4(FragPos, 1.0);
}