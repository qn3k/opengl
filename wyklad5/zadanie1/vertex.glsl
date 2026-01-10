#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord; // NOWY: atrybut UV
layout (location = 2) in vec3 aNormal;

out vec3 vNormal;
out vec3 vFragPos;
out vec2 vTexCoord; // WYJŚCIE: przekazanie do Fragment Shadera

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProj;

void main()
{
    vFragPos = vec3(matModel * vec4(aPos, 1.0));
    vNormal = mat3(transpose(inverse(matModel))) * aNormal;
    vTexCoord = aTexCoord; // Przypisanie UV
    
    gl_Position = matProj * matView * matModel * vec4(aPos, 1.0);
}