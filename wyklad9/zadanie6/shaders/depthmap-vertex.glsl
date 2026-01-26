#version 330 core
layout (location = 0) in vec3 inPosition;
layout (location = 3) in mat4 instanceMatrix; // Macierz dla instancji

uniform mat4 matProj; 
uniform mat4 matView; 
uniform mat4 matModel; // Dla zwykłych obiektow
uniform bool bIsInstanced; // czy instancja

void main()
{
    mat4 model = bIsInstanced ? instanceMatrix : matModel;
    
    gl_Position = matProj * matView * model * vec4(inPosition, 1.0);
}