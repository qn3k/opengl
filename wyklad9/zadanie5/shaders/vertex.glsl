#version 330 core
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoords;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in mat4 instanceMatrix; // kwiaty

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;

uniform mat4 lightProj;
uniform mat4 lightView;

uniform bool bIsInstanced;

void main()
{
    mat4 finalModel = bIsInstanced ? instanceMatrix : matModel;

    FragPos = vec3(finalModel * vec4(inPosition, 1.0));
    // Przekształcamy normalne do world space (uproszczone, bez macierzy normalnych)
    Normal = mat3(transpose(inverse(finalModel))) * inNormal;
    TexCoords = inTexCoords;

    // KLUCZOWE: Obliczamy pozycję fragmentu w przestrzeni światła
    // To trafia do fragment shadera jako FragPosLightSpace
    FragPosLightSpace = lightProj * lightView * vec4(FragPos, 1.0);

    gl_Position = matProj * matView * vec4(FragPos, 1.0);
}